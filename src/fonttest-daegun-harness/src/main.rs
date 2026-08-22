use std::collections::HashMap;
use std::fmt::Write as _;

use daegun::{Font, OutlinePen};

const DAEGUN_VERSION: &str = "1.1.6";

// The suite renders at 1000 ppem and compares with a tolerance of one font design unit.
const PPEM: f64 = 1000.0;

struct SvgPen {
    d: String,
    scale: f64,
    start: (f32, f32),
    closed: bool,
}

impl SvgPen {
    fn new(scale: f64) -> SvgPen {
        SvgPen { d: String::new(), scale, start: (0.0, 0.0), closed: true }
    }
    // FreeType scales into 26.6 fixed point, rounding, and the harness then divides by 64 with C
    // integer division, which truncates toward zero. Both steps matter: -270.996 rounds to -271.0
    // in 26.6 and truncates to -271, where truncating the f64 alone gives -270.
    fn x(&self, v: f32) -> f64 {
        ((v as f64 * self.scale * 64.0).round() / 64.0).trunc()
    }
    fn sep(&mut self) {
        if !self.d.is_empty() {
            self.d.push(' ');
        }
    }
    // FT_Outline_Decompose has no close callback, so the reference closes a contour at the next
    // move or at the end of the glyph.
    fn finish(&mut self) -> &str {
        if !self.closed {
            self.d.push_str(" Z");
            self.closed = true;
        }
        &self.d
    }
}

impl OutlinePen for SvgPen {
    fn move_to(&mut self, x: f32, y: f32) {
        if !self.closed {
            self.d.push_str(" Z");
        }
        self.sep();
        let _ = write!(self.d, "M{},{}", self.x(x), self.x(y));
        self.start = (x, y);
        self.closed = false;
    }
    fn line_to(&mut self, x: f32, y: f32) {
        // A segment landing back on the contour's start is the closing edge, and the reference
        // writes it as Z. Compared before scaling, as the reference compares before dividing.
        if (x, y) == self.start {
            self.d.push_str(" Z");
            self.closed = true;
            return;
        }
        self.sep();
        let _ = write!(self.d, "L{},{}", self.x(x), self.x(y));
        self.closed = false;
    }
    fn quad_to(&mut self, cx: f32, cy: f32, x: f32, y: f32) {
        self.sep();
        let _ = write!(self.d, "Q{},{} {},{}", self.x(cx), self.x(cy), self.x(x), self.x(y));
        self.closed = false;
    }
    fn curve_to(&mut self, c1x: f32, c1y: f32, c2x: f32, c2y: f32, x: f32, y: f32) {
        self.sep();
        let _ = write!(
            self.d,
            "C{},{} {},{} {},{}",
            self.x(c1x), self.x(c1y), self.x(c2x), self.x(c2y), self.x(x), self.x(y)
        );
        self.closed = false;
    }
    fn close(&mut self) {}
}

fn escape(s: &str) -> String {
    s.replace('&', "&amp;").replace('<', "&lt;").replace('>', "&gt;").replace('"', "&quot;")
}

fn parse_variations(arg: &str) -> Vec<(String, f64)> {
    arg.split(';')
        .filter_map(|kv| kv.split_once(':'))
        .filter_map(|(t, v)| v.trim().parse::<f64>().ok().map(|v| (t.trim().to_string(), v)))
        .collect()
}

fn main() {
    let mut font_path = None;
    let mut testcase = None;
    let mut render = None;
    let mut variation = None;
    let mut engine = "daegun".to_string();
    let mut want_version = false;

    for a in std::env::args().skip(1) {
        if a == "--version" {
            want_version = true;
            continue;
        }
        let Some((k, v)) = a.split_once('=') else { continue };
        match k {
            "--font" => font_path = Some(v.to_string()),
            "--testcase" => testcase = Some(v.to_string()),
            "--render" => render = Some(v.to_string()),
            "--variation" => variation = Some(v.to_string()),
            "--engine" => engine = v.to_string(),
            _ => {}
        }
    }

    if want_version {
        println!("{engine}/{DAEGUN_VERSION}");
        return;
    }

    let (Some(font_path), Some(testcase)) = (font_path, testcase) else {
        eprintln!("missing --font or --testcase");
        std::process::exit(2);
    };

    let Some(render) = render else {
        // A test case with no text to render has nothing to output.
        return;
    };

    let bytes = match std::fs::read(&font_path) {
        Ok(b) => b,
        Err(e) => {
            eprintln!("cannot read {font_path}: {e}");
            std::process::exit(2);
        }
    };

    let font = match Font::from_bytes(&bytes) {
        Ok(f) => f,
        Err(e) => {
            eprintln!("cannot parse {font_path}: {e:?}");
            std::process::exit(2);
        }
    };

    let vars = variation.as_deref().map(parse_variations).unwrap_or_default();
    let axes: Vec<(&str, f64)> = vars.iter().map(|(t, v)| (t.as_str(), *v)).collect();

    let upm = font.upm() as f64;
    // Outlines come back in font units; advances are already on a 1000 unit em.
    let outline_scale = PPEM / upm;

    let Some(runs) = font.shape_bidi(&render, &axes, None) else {
        eprintln!("shaping failed");
        std::process::exit(2);
    };

    let mut symbols: Vec<(String, String)> = Vec::new();
    let mut seen: HashMap<u16, Option<String>> = HashMap::new();
    let mut uses: Vec<(String, f64, f64)> = Vec::new();

    let mut pen = 0.0f64;
    let mut advance_width = 0.0f64;

    for run in &runs {
        let r = &run.run;
        for i in 0..r.glyphs.len() {
            let gid = r.glyphs[i];
            let adv = r.advances.get(i).copied().unwrap_or(0.0);
            let (dx, dy) = r.offsets.get(i).copied().unwrap_or((0.0, 0.0));

            let href = seen
                .entry(gid)
                .or_insert_with(|| {
                    let mut p = SvgPen::new(outline_scale);
                    font.outline_glyph_instanced(gid, &axes, &mut p)?;
                    let d = p.finish().trim().to_string();
                    // check.py drops a blank outline and the uses pointing at it, so emitting one
                    // only differs from the reference by an element it would have removed anyway.
                    if d.is_empty() {
                        return None;
                    }
                    let name = font
                        .glyph_name(gid)
                        .unwrap_or_else(|| format!("gid{gid}"));
                    let id = format!("{testcase}.{name}");
                    symbols.push((id.clone(), d));
                    Some(format!("#{id}"))
                })
                .clone();

            if let Some(href) = href {
                uses.push((href, (pen + dx).round(), dy.round()));
            }

            pen += adv;
            advance_width = pen;
        }
    }

    // The suite derives every expected viewBox from `hhea`, uniformly across all 66 of its fonts,
    // so the harness reads the raw accessors. `Font::line_metrics` honors OS/2 USE_TYPO_METRICS and
    // is what daegun uses for line layout, which is what that bit actually governs.
    let ascender = font.ascender() as f64;
    let descender = font.descender() as f64;

    let mut out = String::new();
    let _ = write!(
        out,
        r#"<svg version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 {} {} {}">"#,
        descender,
        advance_width.round(),
        ascender - descender
    );
    for (id, d) in &symbols {
        let _ = write!(
            out,
            r#"<symbol id="{}" overflow="visible"><path d="{}" /></symbol>"#,
            escape(id),
            escape(d)
        );
    }
    for (href, x, y) in &uses {
        let _ = write!(out, r#"<use xlink:href="{}" x="{}" y="{}" />"#, escape(href), x, y);
    }
    out.push_str("</svg>");

    println!("{out}");
}
