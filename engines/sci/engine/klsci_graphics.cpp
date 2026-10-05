/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/lsci_array.h"
#include "sci/engine/lsci_kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/animate.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls16.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/view.h"

namespace Sci {

enum LsciControlType {
	kLsciControlButton = 1,
	kLsciControlText = 2,
	kLsciControlEdit = 3,
	kLsciControlIcon = 4
};

enum {
	kLsciControlEnabled = 1,
	kLsciNoColor = -1
};

reg_t kLsciSetCursor(EngineState *s, int argc, reg_t *argv) {
	switch (argc) {
	case 1:
		if (argv[0].isNull())
			g_sci->_gfxCursor->kernelHide();
		else
			g_sci->_gfxCursor->kernelShow();
		break;
	case 2:
		g_sci->_gfxCursor->kernelSetPos(Common::Point(argv[0].toSint16(), argv[1].toSint16()));
		break;
	default: {
		// A hotspot of -1 keeps the cel's own
		Common::Point *hotspot = nullptr;
		if (argc >= 5 && argv[3].toSint16() != -1)
			hotspot = new Common::Point(argv[3].toSint16(), argv[4].toSint16());
		g_sci->_gfxCursor->kernelSetView(argv[0].toUint16(), argv[1].toUint16(), argv[2].toUint16(), hotspot);
		break;
	}
	}
	return s->r_acc;
}

reg_t kLsciAnimate(EngineState *s, int argc, reg_t *argv) {
	if (argc == 0 || argv[0].isNull())
		return kAnimate(s, argc, argv);

	// The cast is an LSCI list, walked live by doit as LSCITV does; the drawing
	// itself goes through a copy as an SCI list.
	SegManager *segMan = s->_segMan;
	const reg_t cast = argv[0];
	if (argc > 1 && !argv[1].isNull()) {
		const uint16 count = segMan->lookupArray(cast)->size();
		for (uint16 i = 0; i < count && i < segMan->lookupArray(cast)->size(); ++i) {
			const reg_t object = segMan->lookupArray(cast)->IDAt(i);
			if (!(readSelectorValue(segMan, object, SELECTOR(signal)) & kSignalFrozen))
				invokeSelector(s, object, SELECTOR(doit), argc, argv);
			if (s->abortScriptProcessing != kAbortNone)
				return s->r_acc;
		}
	}

	const reg_t list = kNewList(s, 0, nullptr);
	SciArray &castArray = *segMan->lookupArray(cast);
	for (uint16 i = 0; i < castArray.size(); ++i) {
		reg_t node[2] = { list, segMan->newNode(castArray.IDAt(i), castArray.IDAt(i)) };
		kAddToEnd(s, 2, node);
	}
	g_sci->_gfxAnimate->kernelAnimate(list, false, 0, argv);
	return s->r_acc;
}

// The sides of an LSCI Rect object, which are the nsRect selectors
static Common::Rect getLsciRect(reg_t rect) {
	return g_sci->_gfxCompare->getNSRect(rect);
}

reg_t kLsciGraphSaveBits(EngineState *s, int argc, reg_t *argv) {
	return g_sci->_gfxPaint16->kernelGraphSaveBox(getLsciRect(argv[0]), argv[1].toUint16() & GFX_SCREEN_MASK_ALL, false);
}

reg_t kLsciGraphFillRect(EngineState *s, int argc, reg_t *argv) {
	// A map mask, then the colour, priority and control to fill those maps with
	const int16 priority = argc > 3 ? argv[3].toSint16() : -1;
	const int16 control = argc > 4 ? argv[4].toSint16() : -1;
	g_sci->_gfxPaint16->kernelGraphFillBox(getLsciRect(argv[0]), argv[1].toUint16(), argv[2].toSint16(), priority, control);
	return s->r_acc;
}

reg_t kLsciGraphShowBits(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPaint16->kernelGraphUpdateBox(getLsciRect(argv[0]));
	return s->r_acc;
}

reg_t kLsciGraphRedrawBox(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPaint16->kernelGraphRedrawBox(getLsciRect(argv[0]));
	return s->r_acc;
}

reg_t kLsciTextSize(EngineState *s, int argc, reg_t *argv) {
	// The size goes into a Rect object, from 0, 0
	const Common::String text = getLsciString(s->_segMan, argv[1]);
	int16 width = 0;
	int16 height = 0;
	if (!text.empty())
		g_sci->_gfxText16->kernelTextSize(text.c_str(), 0, argv[2].toUint16(), argc > 3 ? argv[3].toUint16() : 0, &width, &height);
	g_sci->_gfxCompare->setNSRect(argv[0], Common::Rect(width, height));
	return s->r_acc;
}

reg_t kLsciDisplay(EngineState *s, int argc, reg_t *argv) {
	// The text may also be a Str object, or 0 for none; LSCI has no text resources
	if (argv[0].isNull())
		return g_sci->_gfxPaint16->kernelDisplay("", 0, argc - 1, argv + 1);
	if (s->_segMan->isObject(argv[0]))
		argv[0] = readSelector(s->_segMan, argv[0], SELECTOR(data));
	return kDisplay(s, argc, argv);
}

reg_t kLsciCelRect(EngineState *s, int argc, reg_t *argv) {
	// The rect of a cel placed by its origin, into a Rect object; 128 is full scale
	GfxView *view = g_sci->_gfxCache->getView(argv[1].toUint16());
	const int16 z = argc > 6 ? argv[6].toSint16() : 0;
	const int16 scaleX = argc > 7 ? argv[7].toSint16() : 128;
	const int16 scaleY = argc > 8 ? argv[8].toSint16() : scaleX;
	Common::Rect rect;
	if (scaleX == 128 && scaleY == 128)
		view->getCelRect(argv[2].toSint16(), argv[3].toSint16(), argv[4].toSint16(), argv[5].toSint16(), z, rect);
	else
		view->getCelScaledRect(argv[2].toSint16(), argv[3].toSint16(), argv[4].toSint16(), argv[5].toSint16(), z, scaleX, scaleY, rect);
	g_sci->_gfxCompare->setNSRect(argv[0], rect);
	return s->r_acc;
}

reg_t kLsciDrawCel(EngineState *s, int argc, reg_t *argv) {
	// Drawn from its top left corner; the colour table in argument 7 is not supported
	const int16 priority = argc > 5 ? argv[5].toSint16() : -1;
	const uint16 scaleX = argc > 7 ? argv[7].toUint16() : 128;
	const uint16 scaleY = argc > 8 ? argv[8].toUint16() : scaleX;
	g_sci->_gfxPaint16->kernelDrawCel(argv[0].toUint16(), argv[1].toSint16(), argv[2].toSint16(),
		argv[3].toUint16(), argv[4].toUint16(), priority, 0, scaleX, scaleY, false, NULL_REG);
	return s->r_acc;
}

reg_t kLsciWindowNew(EngineState *s, int argc, reg_t *argv) {
	// The scripts draw the window themselves, so ScummVM's window is a plain user
	// style port that only keeps what was under the frame rect.
	SegManager *segMan = s->_segMan;
	const reg_t window = argv[0];
	const Common::Rect frame = getLsciRect(readSelector(segMan, window, SELECTOR(frameRect)));
	return g_sci->_gfxPorts->kernelNewWindow(getLsciRect(window), frame, SCI_WINDOWMGR_STYLE_USER,
		readSelectorValue(segMan, window, SELECTOR(priority)), 0, 255, nullptr);
}

reg_t kLsciSetPort(EngineState *s, int argc, reg_t *argv) {
	g_sci->_gfxPorts->kernelSetActive(argc > 0 ? argv[0].toUint16() : 0);
	return s->r_acc;
}

reg_t kLsciDrawControl(EngineState *s, int argc, reg_t *argv) {
	// Buttons and texts are boxed text; a disabled button is greyed out
	SegManager *segMan = s->_segMan;
	const reg_t control = argv[0];
	const int16 type = readSelectorValue(segMan, control, SELECTOR(type));
	const int16 state = readSelectorValue(segMan, control, SELECTOR(state));
	const Common::Rect rect = getLsciRect(control);
	const GuiResourceId font = readSelectorValue(segMan, control, SELECTOR(font));
	const Common::String text = getLsciString(segMan, readSelector(segMan, control, SELECTOR(text)));

	switch (type) {
	case kLsciControlButton:
	case kLsciControlText: {
		Port *port = g_sci->_gfxPorts->getPort();
		const Port savedPort = *port;
		const int16 color = readSelectorValue(segMan, control, SELECTOR(color));
		const int16 back = readSelectorValue(segMan, control, SELECTOR(back));
		if (color != kLsciNoColor)
			port->penClr = color;
		if (back != kLsciNoColor)
			port->backClr = back;
		port->greyedOutput = type == kLsciControlButton && !(state & kLsciControlEnabled);
		g_sci->_gfxControls16->kernelDrawText(rect, control, text.c_str(), 0, font,
			readSelectorValue(segMan, control, SELECTOR(mode)), state, false);
		port->penClr = savedPort.penClr;
		port->backClr = savedPort.backClr;
		port->greyedOutput = savedPort.greyedOutput;
		break;
	}
	case kLsciControlEdit:
		g_sci->_gfxControls16->kernelDrawTextEdit(rect, control, text.c_str(), 0, font,
			readSelectorValue(segMan, control, SELECTOR(mode)), state,
			readSelectorValue(segMan, control, SELECTOR(cursor)), readSelectorValue(segMan, control, SELECTOR(max)), false);
		break;
	case kLsciControlIcon:
		g_sci->_gfxControls16->kernelDrawIcon(rect, control, readSelectorValue(segMan, control, SELECTOR(view)),
			readSelectorValue(segMan, control, SELECTOR(loop)), readSelectorValue(segMan, control, SELECTOR(cel)), -1, state, false);
		break;
	default:
		warning("kDrawControl: control type %d is not implemented", type);
		break;
	}
	return s->r_acc;
}

reg_t kLsciHiliteControl(EngineState *s, int argc, reg_t *argv) {
	const Common::Rect rect = getLsciRect(argv[0]);
	g_sci->_gfxPaint16->invertRect(rect);
	g_sci->_gfxPaint16->bitsShow(rect);
	return s->r_acc;
}

reg_t kLsciEditControl(EngineState *s, int argc, reg_t *argv) {
	// 1 for anything but an edit control, which the event goes to
	if (argv[0].isNull() || readSelectorValue(s->_segMan, argv[0], SELECTOR(type)) != kLsciControlEdit)
		return make_reg(0, 1);
	return kEditControl(s, argc, argv);
}

reg_t kLsciCanBeHere(EngineState *s, int argc, reg_t *argv) {
	// 1 when the base rect is clear of illegal controls and of the cast's base rects
	SegManager *segMan = s->_segMan;
	const reg_t object = argv[0];
	const Common::Rect rect = getLsciRect(readSelector(segMan, object, SELECTOR(baseRect)));
	const uint16 illegalBits = readSelectorValue(segMan, object, SELECTOR(illegalBits));
	if (g_sci->_gfxCompare->kernelOnControl(GFX_SCREEN_MASK_CONTROL, rect) & illegalBits)
		return NULL_REG;

	const uint16 ignored = kSignalIgnoreActor | kSignalRemoveView;
	if (argc < 2 || argv[1].isNull() || (readSelectorValue(segMan, object, SELECTOR(signal)) & ignored))
		return make_reg(0, 1);

	SciArray &cast = *segMan->lookupArray(argv[1]);
	for (uint16 i = 0; i < cast.size(); ++i) {
		const reg_t other = cast.IDAt(i);
		if (other == object || (readSelectorValue(segMan, other, SELECTOR(signal)) & (ignored | kSignalNoUpdate)))
			continue;
		const Common::Rect otherRect = getLsciRect(readSelector(segMan, other, SELECTOR(baseRect)));
		if (otherRect.right > rect.left && otherRect.left < rect.right &&
			otherRect.bottom > rect.top && otherRect.top < rect.bottom)
			return NULL_REG;
	}
	return make_reg(0, 1);
}

reg_t kLsciOnControl(EngineState *s, int argc, reg_t *argv) {
	// A map mask with a Rect object or a point
	Common::Rect rect;
	if (argc == 2) {
		rect = getLsciRect(argv[1]);
	} else {
		rect.left = argv[1].toSint16();
		rect.top = argv[2].toSint16();
		rect.right = rect.left + 1;
		rect.bottom = rect.top + 1;
	}
	return make_reg(0, g_sci->_gfxCompare->kernelOnControl(argv[0].toUint16(), rect));
}

reg_t kLsciNumLoops(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxCache->kernelViewGetLoopCount(argv[0].toUint16()));
}

reg_t kLsciNumCels(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, g_sci->_gfxCache->kernelViewGetCelCount(argv[0].toUint16(), argv[1].toSint16()));
}

reg_t kLsciBaseRectSet(EngineState *s, int argc, reg_t *argv) {
	// The cel rect, from yStep above y down to y, into the view's baseRect object
	SegManager *segMan = s->_segMan;
	const reg_t object = argv[0];
	const reg_t baseRect = readSelector(segMan, object, SELECTOR(baseRect));
	if (baseRect.isNull())
		return s->r_acc;

	const int16 x = readSelectorValue(segMan, object, SELECTOR(x));
	const int16 y = readSelectorValue(segMan, object, SELECTOR(y));
	const int16 z = readSelectorValue(segMan, object, SELECTOR(z));
	GfxView *view = g_sci->_gfxCache->getView(readSelectorValue(segMan, object, SELECTOR(view)));
	Common::Rect celRect;
	view->getCelRect(readSelectorValue(segMan, object, SELECTOR(loop)), readSelectorValue(segMan, object, SELECTOR(cel)), x, y, z, celRect);

	writeSelectorValue(segMan, baseRect, SELECTOR(top), y - readSelectorValue(segMan, object, SELECTOR(yStep)));
	writeSelectorValue(segMan, baseRect, SELECTOR(left), celRect.left);
	writeSelectorValue(segMan, baseRect, SELECTOR(bottom), y + 1);
	writeSelectorValue(segMan, baseRect, SELECTOR(right), celRect.right);
	return s->r_acc;
}

} // End of namespace Sci
