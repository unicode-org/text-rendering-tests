/* Copyright 2019 Unicode Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

extern "C" {
#include <ft2build.h>
#include FT_ADVANCES_H
#include FT_FREETYPE_H
#include FT_MULTIPLE_MASTERS_H
#include FT_TRUETYPE_TABLES_H

#include <SheenBidi.h>
#include <SheenFigure.h>
}

#include "fonttest/freestack_path.h"
#include "fonttest/tehreerstack_line.h"

namespace fonttest {

static SFFontRef DeriveVariableFont(FT_Face face, SFFontRef font) {
  SFFontRef derivedFont = font;
  FT_MM_Var *variation;

  if (FT_Get_MM_Var(face, &variation) == FT_Err_Ok) {
    FT_UInt coordCount = variation->num_axis;
    FT_Fixed fixedCoords[coordCount];
    SFInt16 coordArray[coordCount];

    if (FT_Get_Var_Blend_Coordinates(face, coordCount, fixedCoords) == FT_Err_Ok) {
      for (FT_UInt i = 0; i < coordCount; i++) {
        coordArray[i] = static_cast<SFInt16>(fixedCoords[i] >> 2);
      }

      derivedFont = SFFontCreateWithVariationCoordinates(font, face, coordArray, coordCount);
      SFFontRelease(font);
    }

    FT_Done_MM_Var(face->glyph->library, variation);
  }

  return derivedFont;
}

static SFFontRef CreateFontInstance(FT_Face face) {
  SFFontProtocol protocol;
  protocol.finalize = nullptr;
  protocol.loadTable = [](void *object, SFTag tag, SFUInt8 *buffer, SFUInteger *length) {
    FT_Face face = reinterpret_cast<FT_Face>(object);
    FT_ULong size = 0;
    FT_Load_Sfnt_Table(face, tag, 0, buffer, length ? &size : nullptr);

    if (length) {
      *length = size;
    }
  };
  protocol.getGlyphIDForCodepoint = [](void *object, SFCodepoint codepoint) {
    FT_Face face = reinterpret_cast<FT_Face>(object);
    FT_UInt glyphID = FT_Get_Char_Index(face, codepoint);

    return static_cast<SFGlyphID>(glyphID);
  };
  protocol.getAdvanceForGlyph = [](void *object, SFFontLayout fontLayout, SFGlyphID glyphID) {
    FT_Face face = reinterpret_cast<FT_Face>(object);
    FT_Fixed advance = 0;
    FT_Get_Advance(face, glyphID, FT_LOAD_NO_SCALE, &advance);

    return static_cast<SFInt32>(advance);
  };

  return DeriveVariableFont(face, SFFontCreateWithProtocol(&protocol, face));
}

static void PopulateScriptArray(SBScript *scriptArr, const SBCodepointSequence *uniSeq) {
  SBScriptLocatorRef scriptLoc = SBScriptLocatorCreate();
  const SBScriptAgent *scriptAgent = SBScriptLocatorGetAgent(scriptLoc);
  SBScriptLocatorLoadCodepoints(scriptLoc, uniSeq);

  while (SBScriptLocatorMoveNext(scriptLoc)) {
    for (size_t i = 0; i < scriptAgent->length; i++) {
      scriptArr[scriptAgent->offset + i] = scriptAgent->script;
    }
  }

  SBScriptLocatorRelease(scriptLoc);
}

static void InsertGlyphInfos(SFAlbumRef album, SFTextDirection direction, double ppem, std::vector<GlyphInfo> &glyphInfos) {
  SFUInteger len = SFAlbumGetGlyphCount(album);
  const SFGlyphID *glyphIDs = SFAlbumGetGlyphIDsPtr(album);
  const SFPoint *offsets = SFAlbumGetGlyphOffsetsPtr(album);
  const SFInt32 *advances = SFAlbumGetGlyphAdvancesPtr(album);

  SFBoolean rev = (direction == SFTextDirectionRightToLeft);
  SFUInteger inc = (rev ? -1 : 1);

  for (SFInteger i = (rev ? len - 1 : 0); i >= 0 && i < len; i += inc) {
    GlyphInfo glyphInfo = {
      glyphIDs[i],
      offsets[i].x * ppem,
      offsets[i].y * ppem,
      advances[i] * ppem
    };

    glyphInfos.push_back(glyphInfo);
  }
}

TehreerStackLine::TehreerStackLine(
    const std::string& text, const std::string& textLanguage,
    FT_Face font, double fontSize)
  : font_(font), fontSize_(fontSize) {
  sfFont_ = CreateFontInstance(font);

  const double ppem = fontSize / font->units_per_EM;

  const char *txtBuf = text.c_str();
  SBUInteger txtLen = text.length();
  SBCodepointSequence uniSeq = { SBStringEncodingUTF8, (void *)txtBuf, txtLen };

  SBScript *scriptArr = new SBScript[text.length()];
  PopulateScriptArray(scriptArr, &uniSeq);

  SFArtistRef artist = SFArtistCreate();
  SFSchemeRef scheme = SFSchemeCreate();

  SBAlgorithmRef bidiAlgo = SBAlgorithmCreate(&uniSeq);
  SBUInteger paraStart = 0;

  while (paraStart != txtLen) {
    SBParagraphRef paragraph = SBAlgorithmCreateParagraph(bidiAlgo, paraStart, txtLen, SBLevelDefaultLTR);
    SBUInteger paraLen = SBParagraphGetLength(paragraph);

    SBLineRef line = SBParagraphCreateLine(paragraph, 0, paraLen);
    const SBRun *runArr = SBLineGetRunsPtr(line);
    SBUInteger runCount = SBLineGetRunCount(line);

    for (SBUInteger runIdx = 0; runIdx < runCount; runIdx++) {
      SBLevel runLevel = runArr[runIdx].level;
      SBUInteger runStart = runArr[runIdx].offset;
      SBUInteger runEnd = runStart + runArr[runIdx].length;

      SBScript scriptVal = scriptArr[runStart];
      SBUInteger scriptIdx = runStart;

      while (scriptIdx < runEnd) {
        void *shapeBuf = (void *)&txtBuf[scriptIdx];
        SBUInteger shapeStart = scriptIdx;

        for (scriptIdx += 1; scriptIdx < runEnd; scriptIdx++) {
          if (scriptArr[scriptIdx] != scriptVal) {
            break;
          }
        }

        SBUInteger shapeLen = scriptIdx - shapeStart;
        SFTag scriptTag = SBScriptGetOpenTypeTag(scriptVal);
        SFTextDirection scriptDir = SFScriptGetDefaultDirection(scriptTag);
        SFTextMode textMode = SFTextModeForward;

        if (((runLevel & 1) && scriptDir == SFTextDirectionLeftToRight)
            | (!(runLevel & 1) && scriptDir == SFTextDirectionRightToLeft)) {
          textMode = SFTextModeBackward;
        }

        SFSchemeSetFont(scheme, sfFont_);
        SFSchemeSetScriptTag(scheme, scriptTag);
        SFSchemeSetLanguageTag(scheme, SFTagMake('d', 'f', 'l', 't'));

        SFPatternRef pattern = SFSchemeBuildPattern(scheme);
        SFAlbumRef album = SFAlbumCreate();

        SFArtistSetPattern(artist, pattern);
        SFArtistSetString(artist, SFStringEncodingUTF8, shapeBuf, shapeLen);
        SFArtistSetTextDirection(artist, scriptDir);
        SFArtistSetTextMode(artist, textMode);
        SFArtistFillAlbum(artist, album);

        InsertGlyphInfos(album, scriptDir, ppem, glyphInfos);

        SFAlbumRelease(album);
        SFPatternRelease(pattern);
      }
    }

    SBLineRelease(line);
    SBParagraphRelease(paragraph);

    paraStart += paraLen;
  }

  SFSchemeRelease(scheme);
  SFArtistRelease(artist);

  delete [] scriptArr;
}

TehreerStackLine::~TehreerStackLine() {
  SFFontRelease(sfFont_);
}

bool TehreerStackLine::RenderSVG(const std::string& idPrefix, std::string* svg) {
  svg->clear();

  const double ascender = fontSize_ *
      (static_cast<double>(font_->ascender) /
       static_cast<double>(font_->units_per_EM));
  const double descender = fontSize_ *
      (static_cast<double>(font_->descender) /
       static_cast<double>(font_->units_per_EM));

  size_t len = glyphInfos.size();

  std::string symbols;
  std::map<int, std::string> glyphNames;
  for (size_t i = 0; i < len; ++i) {
    const GlyphInfo &glyph = glyphInfos[i];
    if (glyphNames.find(glyph.glyphID) != glyphNames.end()) {
      continue;
    }

    char glyphName[512];
    FT_Error error = FT_Get_Glyph_Name(font_, glyph.glyphID, glyphName, sizeof(glyphName));
    if (error || *glyphName == '\0') {
      snprintf(glyphName, sizeof(glyphName), "gid%u", glyph.glyphID);
    }
    glyphNames[glyph.glyphID] = std::string(glyphName);

    error = FT_Load_Glyph(font_, glyph.glyphID, FT_LOAD_NO_HINTING|FT_LOAD_NO_BITMAP);
    if (error) {
      std::cerr << "FT_Load_Glyph() failed; error: " << error << std::endl;
      exit(1);
    }

    if (!font_->glyph) {
      std::cerr << "FT_Load_Glyph() did not load a glyph" << std::endl;
      exit(1);
    }

    symbols.append("  <symbol id=\"");
    symbols.append(idPrefix);
    symbols.append(".");
    symbols.append(glyphName);
    symbols.append("\" overflow=\"visible\"><path d=\"");
    FT_Vector transform;
    transform.x = transform.y = 0;
    FreeTypePathConverter converter(transform);
    symbols.append(converter.Convert(&font_->glyph->outline));
    symbols.append("\"/></symbol>\n");
  }

  std::string uses;
  double penX = 0;
  for (size_t i = 0; i < len; i++) {
    const GlyphInfo &glyph = glyphInfos[i];
    const double glyphX = penX + glyph.xOffset;
    const double glyphY = glyph.yOffset;
    char buffer[1024];
    snprintf(buffer, sizeof(buffer),
             "  <use xlink:href=\"#%s.%s\" x=\"%ld\" y=\"%ld\"/>\n",
             idPrefix.c_str(), glyphNames[glyph.glyphID].c_str(),
             lround(glyphX), lround(glyphY));
    uses.append(buffer);
    penX += glyph.advance;
  }

  svg->clear();
  svg->append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
              "<svg version=\"1.1\"\n"
              "    xmlns=\"http://www.w3.org/2000/svg\"\n"
              "    xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
              "    viewBox=\"");
  char viewBox[200];
  snprintf(viewBox, sizeof(viewBox), "%ld %ld %ld %ld",
           0L, lround(descender), lround(penX),
           lround(ascender - descender));
  svg->append(viewBox);
  svg->append("\">\n");

  if (false) {
    char buffer[200];
    snprintf(buffer, sizeof(buffer),
	     "  <rect fill=\"none\" stroke=\"blue\" stroke-width=\"10\" "
	     "x=\"%ld\" y=\"%ld\" width=\"%ld\" height=\"%ld\"/>\n",
	     0L, lround(descender), lround(penX),
	     lround(ascender - descender));
    svg->append(buffer);
  }

  svg->append(symbols);
  svg->append(uses);
  svg->append("</svg>\n");
  return true;
}

}  // namespace fonttest
