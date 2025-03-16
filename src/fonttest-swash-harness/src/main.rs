use std::{
    borrow::Cow,
    collections::HashMap,
    error::Error,
    ffi::{OsStr, OsString},
    fmt::Write,
    io::Cursor,
    path::PathBuf,
};

use lexopt::prelude::*;
use swash::{
    FontRef, Setting,
    scale::{ScaleContext, outline::Outline},
    shape::{Direction, ShapeContext},
    tag_from_str_lossy,
    text::Script,
    zeno::{Command, PathData, Vector},
};
use unicode_bidi::{BidiInfo, Level};

#[derive(Debug)]
enum Args {
    PrintVersion {
        engine: OsString,
    },
    Render {
        // The driver script always passes --engine, even though we don't do anything with it unless printing the
        // version.
        _engine: OsString,
        font: OsString,
        testcase: OsString,
        render: Option<OsString>,
        variation: Option<OsString>,
    },
}

fn parse_args() -> Result<Args, lexopt::Error> {
    let mut parser = lexopt::Parser::from_env();

    let mut font = None;
    let mut testcase = None;
    let mut render = None;
    let mut variation = None;
    let mut engine = None;
    let mut version = false;

    while let Some(arg) = parser.next()? {
        match arg {
            Long("font") => {
                font = Some(parser.value()?);
            }
            Long("testcase") => {
                testcase = Some(parser.value()?);
            }
            Long("engine") => {
                engine = Some(parser.value()?);
            }
            Long("render") => {
                render = Some(parser.value()?);
            }
            Long("variation") => {
                variation = Some(parser.value()?);
            }
            Long("version") => {
                version = true;
            }
            _ => return Err(arg.unexpected()),
        }
    }

    if version {
        Ok(Args::PrintVersion {
            engine: engine.ok_or("missing engine argument")?,
        })
    } else {
        Ok(Args::Render {
            _engine: engine.ok_or("missing engine argument")?,
            font: font.ok_or("missing font argument")?,
            testcase: testcase.ok_or("missing testcase argument")?,
            render,
            variation,
        })
    }
}

/// Parse the `--variations` command-line argument (key:value pairs separated by semicolons)
fn parse_variations(variations_arg: &OsStr) -> Result<Vec<Setting<f32>>, Box<dyn Error>> {
    let mut variations = Vec::new();
    let variations_str = variations_arg.to_string_lossy();
    for var_kv in variations_str.split(";") {
        let (tag, value) = var_kv
            .split_once(":")
            .ok_or_else(|| format!("invalid variation string: {var_kv}"))?;
        let value: f32 = value.parse()?;
        let tag = tag_from_str_lossy(tag);
        variations.push(Setting { tag, value });
    }
    Ok(variations)
}

/// Convert a swash/zeno [`Outline`] to an SVG `<path>`'s `d` attribute.
fn outline_to_svg_path(outline: Outline, floor_coords: bool) -> Result<String, std::fmt::Error> {
    let maybe_floor_coords = |vectors: &mut [&mut Vector]| {
        if floor_coords {
            for v in vectors {
                **v = v.floor();
            }
        }
    };
    let mut d = String::new();
    for command in outline.path().commands() {
        match command {
            Command::MoveTo(mut vector) => {
                maybe_floor_coords(&mut [&mut vector]);
                write!(d, "M{},{} ", vector.x, vector.y)?
            }
            Command::LineTo(mut vector) => {
                maybe_floor_coords(&mut [&mut vector]);
                write!(d, "L{},{} ", vector.x, vector.y)?
            }
            Command::CurveTo(mut vector, mut vector1, mut vector2) => {
                maybe_floor_coords(&mut [&mut vector, &mut vector1, &mut vector2]);
                write!(
                    d,
                    "C{},{} {},{} {},{} ",
                    vector.x, vector.y, vector1.x, vector1.y, vector2.x, vector2.y
                )?;
            }
            Command::QuadTo(mut vector, mut vector1) => {
                maybe_floor_coords(&mut [&mut vector, &mut vector1]);
                write!(d, "Q{},{} {},{} ", vector.x, vector.y, vector1.x, vector1.y)?;
            }
            Command::Close => write!(d, "Z ")?,
        };
    }
    // Remove the last trailing space
    d.pop();
    Ok(d)
}

fn main() -> Result<(), Box<dyn Error>> {
    let args = parse_args()?;

    // swash doesn't export its own version. We need to hardcode it.
    let swash_version = "0.2.1";
    // Pixels per em. The tests want this to always be 1000. If set to 0, everything will be in font units.
    let ppem = 1000.0;
    // The tests seem to want all coordinates to be floored.
    let floor_coords = true;

    match args {
        Args::PrintVersion { engine } => {
            println!("{}/{swash_version}", engine.as_os_str().to_string_lossy())
        }
        Args::Render {
            font,
            testcase,
            render,
            variation,
            ..
        } => {
            let testcase = testcase.to_str().ok_or("testcase name is invalid UTF-8")?;

            let variations = if let Some(variations_arg) = variation {
                parse_variations(&variations_arg)?
            } else {
                Default::default()
            };

            let font_data = std::fs::read(PathBuf::from(&font))?;
            let font_ref = FontRef::from_index(&font_data, 0).ok_or("could not read font")?;
            let ttfp_face = ttf_parser::Face::parse(&font_data, 0)?;

            let mut context = ShapeContext::new();

            let mut scale_ctx = ScaleContext::new();
            let mut scaler = scale_ctx
                .builder(font_ref)
                .variations(variations.iter().copied())
                .size(ppem)
                .build();

            let mut symbols = Vec::new();
            let mut scaled_glyphs = HashMap::<u16, Option<String>>::new();
            let mut outline_refs = Vec::new();

            let mut advance = 0.0;
            let mut advance_width = 0.0;

            let maybe_floor = |n: f32| {
                if floor_coords { n.floor() } else { n }
            };

            if let Some(render) = render {
                let render = render.to_str().ok_or("test case is invalid UTF-8")?;
                let bidi_info = BidiInfo::new(render, None);
                let mut run = String::new();
                let mut prev_level = Level::ltr();
                let mut prev_script = Script::Common;

                let mut shape_run = |text: String, is_rtl, script| {
                    let direction = if is_rtl {
                        Direction::RightToLeft
                    } else {
                        Direction::LeftToRight
                    };
                    let mut shaper = context
                        .builder(font_ref)
                        .variations(variations.iter().copied())
                        .size(ppem)
                        .direction(direction)
                        .script(script)
                        .insert_dotted_circles(true)
                        .build();

                    shaper.add_str(&text);

                    let mut run_advance = 0.0;
                    let mut glyphs = Vec::new();
                    shaper.shape_with(|cluster| {
                        if is_rtl {
                            for glyph in cluster.glyphs.iter().rev() {
                                glyphs.push(glyph.clone());
                            }
                        } else {
                            for glyph in cluster.glyphs {
                                glyphs.push(glyph.clone());
                            }
                        }
                        run_advance += cluster.advance();
                    });
                    if is_rtl {
                        glyphs.reverse();
                    }
                    for glyph in glyphs {
                        let symbol_href = scaled_glyphs
                            .entry(glyph.id)
                            .or_insert_with(|| {
                                // First time seeing this character. Outline it and add a <symbol> element.
                                let path_commands =
                                    scaler.scale_outline(glyph.id).map(|outline| {
                                        outline_to_svg_path(outline, floor_coords).unwrap()
                                    })?;

                                // `glyph_name` is an unstable function used for testing
                                // (https://github.com/dfrg/swash/issues/6)
                                // but it's the only way to get the glyph names we need
                                // let glyph_name = font_ref.glyph_name(glyph.id);

                                // Swash only supports glyph names in the `post` table, but some tests require glyph
                                // names from the `cff` table. To avoid spurious failures, use ttf_parser for glyph
                                // names only until https://github.com/googlefonts/fontations/pull/1387 makes it in and
                                // swash can use it.
                                let glyph_name =
                                    ttfp_face.glyph_name(ttf_parser::GlyphId(glyph.id));

                                // Fall back to "gidXXX" if no glyph name is found
                                let glyph_name = glyph_name
                                    .map(Cow::Borrowed)
                                    .unwrap_or_else(|| Cow::Owned(format!("gid{}", glyph.id)));

                                let symbol_name = format!("{}.{}", testcase, glyph_name);
                                let symbol_ref_name = format!("#{}.{}", testcase, glyph_name);
                                symbols.push((symbol_name, path_commands));

                                // Then, return the `xlink:href` to the <symbol> we just added.
                                Some(symbol_ref_name)
                            })
                            .clone();

                        let Some(symbol_href) = symbol_href else {
                            continue;
                        };

                        outline_refs.push((symbol_href, advance, glyph.x, glyph.y));
                        advance += glyph.advance;
                        if !glyph.info.is_mark() {
                            advance_width = advance;
                        }
                    }
                };

                for ((properties, _boundary), (byte_index, character)) in
                    swash::text::analyze(render.chars()).zip(render.char_indices())
                {
                    let level = bidi_info.levels[byte_index];
                    let mut script = properties.script();
                    if script == Script::Common
                        || script == Script::Unknown
                        || script == Script::Inherited
                    {
                        script = prev_script;
                    } else if prev_script == Script::Common
                        || prev_script == Script::Unknown
                        || prev_script == Script::Inherited
                    {
                        // Treat starting characters' script as the first "real" script encountered
                        prev_script = script;
                    }
                    if level != prev_level || script != prev_script {
                        if !run.is_empty() {
                            shape_run(std::mem::take(&mut run), prev_level.is_rtl(), prev_script);
                        }
                        prev_level = level;
                        prev_script = script;
                    }
                    run.push(character);
                }
                if !run.is_empty() {
                    shape_run(std::mem::take(&mut run), prev_level.is_rtl(), prev_script);
                }

                let default_shaper = context
                    .builder(font_ref)
                    .variations(variations.iter().copied())
                    .size(ppem)
                    .build();
                let metrics = default_shaper.metrics();

                let mut svg_out =
                    quick_xml::Writer::new_with_indent(Cursor::new(Vec::<u8>::new()), b' ', 4);

                let metrics_scale = if ppem == 0.0 {
                    metrics.units_per_em as f32
                } else {
                    1.0
                };
                let viewbox = [
                    0.0,
                    maybe_floor(-metrics.descent * metrics_scale),
                    maybe_floor(advance_width),
                    maybe_floor((metrics.ascent + metrics.descent) * metrics_scale),
                ];

                svg_out
                    .create_element("svg")
                    .with_attributes([
                        ("version", "1.1"),
                        ("xmlns", "http://www.w3.org/2000/svg"),
                        ("xmlns:xlink", "http://www.w3.org/1999/xlink"),
                        (
                            "viewBox",
                            format!(
                                "{} {} {} {}",
                                viewbox[0], viewbox[1], viewbox[2], viewbox[3]
                            )
                            .as_str(),
                        ),
                    ])
                    .write_inner_content(|w| {
                        for (sym_id, sym_path) in symbols.iter() {
                            w.create_element("symbol")
                                .with_attributes([("id", sym_id.as_str()), ("overflow", "visible")])
                                .write_inner_content(|w| {
                                    w.create_element("path")
                                        .with_attribute(("d", sym_path.as_str()))
                                        .write_empty()?;
                                    Ok(())
                                })?;
                        }

                        for (glyph_id, advance, x, y) in &outline_refs {
                            w.create_element("use")
                                .with_attributes([
                                    ("xlink:href", glyph_id.as_str()),
                                    ("x", maybe_floor(x + advance).to_string().as_str()),
                                    ("y", maybe_floor(*y).to_string().as_str()),
                                ])
                                .write_empty()?;
                        }

                        Ok(())
                    })?;

                let svg_str = String::from_utf8(svg_out.into_inner().into_inner())?;
                println!("{}", svg_str);
            }
        }
    }

    Ok(())
}
