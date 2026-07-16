/*
 * Surge XT - a free and open source hybrid synthesizer,
 * built by Surge Synth Team
 *
 * Learn more at https://surge-synthesizer.github.io/
 *
 * Copyright 2018-2024, various authors, as described in the VCS
 * history and in the readme file in the root of this repository.
 *
 * Surge XT is released under the GNU General Public Licence v3
 * or later (GPL-3.0-or-later). The license is found in the "LICENSE"
 * file in the root of this repository, or at
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * Surge was a commercial product from 2004-2018, copyright and ownership
 * held by Claes Johanson at Vember Audio during that period.
 * Claes made Surge open source in September 2018.
 *
 * All source for Surge XT is available at
 * https://github.com/surge-synthesizer/surge
 */

#pragma once

#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "juce_gui_basics/juce_gui_basics.h"

class SurgeStorage;
struct MSEGStorage;
struct LFOStorage;

namespace Surge
{
namespace Overlays
{

/*
 * The keyboard model for the accessible MSEG editor. A cursor always sits on
 * either a node or a segment control point; keys move the cursor along the
 * MSEG and edit the storage through the same Surge::MSEG API the mouse uses.
 * The handler owns no UI; the hosting canvas wires it up through Callbacks and
 * forwards keyPressed to processKey when the Accessible MSEG Editor option is on.
 */
struct MSEGAccessibleKeyboardHandler
{
    enum struct Part
    {
        NODE,
        CONTROL_POINT
    };

    enum struct Mode
    {
        CURSOR,
        MANIPULATE,
        SELECTION,
        GROUP_MANIPULATE
    };

    struct Callbacks
    {
        std::function<void(const std::string &)> announce;
        std::function<void()> prepareForUndo;
        std::function<void()> pushToUndo;
        std::function<void()> pushToUndoIfDirty;
        // arg is the active segment for pan constraints, or -1
        std::function<void(int)> modelChanged;
        std::function<void(int)> openContextMenuForSegment;
        // segment, MSEGCanvas::SegmentProps as an int, or -1 for the 2D control
        // point chooser
        std::function<void(int, int)> showTypein;
        // MSEGCanvas::TimeEdit as an int (0 single, 1 shift, 2 draw)
        std::function<int()> getTimeEditMode;
        // node selection, in the same index space as the mouse lasso
        // (node i = start of segment i; the final endpoint is not selectable)
        std::function<std::vector<int>()> getSelection;
        std::function<void(const std::vector<int> &)> setSelection;
        std::function<void()> repaint;
    };

    SurgeStorage *storage{nullptr};
    MSEGStorage *ms{nullptr};
    LFOStorage *lfodata{nullptr};
    Callbacks cb;

    Part part{Part::NODE};
    int index{0};
    float cursorY{0.f};
    float rememberedTime{0.f};
    Mode mode{Mode::CURSOR};

    // set while the handler applies its own edit, so revalidateCursor calls
    // triggered by that edit's modelChanged don't double-announce
    bool inFlightEdit{false};

    bool processKey(const juce::KeyPress &key);
    void revalidateCursor(bool announceIfMoved);
    void cancelModes();
    void announceFocus();
    void announceEditorState();

    // geometry of the cursor for the canvas's visual highlight
    int numNodes() const;
    float nodeTime(int i) const;
    float nodeValue(int i) const;
    bool controlPointUsable(int seg) const;
    bool controlPointIs2D(int seg) const;
    float controlPointTime(int seg) const;
    float controlPointValue(int seg) const;
    std::pair<float, float> cursorPosition() const;
    int segmentForCursor() const;

  private:
    bool processCursorKey(const juce::KeyPress &key);
    bool processManipulateKey(const juce::KeyPress &key);
    bool processSelectionKey(const juce::KeyPress &key);
    bool processGroupKey(const juce::KeyPress &key);

    void enterManipulate();
    void exitManipulate(bool announce);
    void enterSelection();
    void exitSelection(bool clear);
    void enterGroup();
    void exitGroup(bool announce);

    void moveCursor(int direction, bool skipControlPoints);
    void placeCursorOnNode(int node);
    void placeCursorOnControlPoint(int seg);

    void addNode(bool append);
    void deleteNode(bool removeSegment);
    void resetCP();
    void nudgeNodeX(int node, float dx, float snap, bool announceResult);
    void nudgeNodeY(int node, float dy, float snapRes, bool announceResult);
    void nudgeControlPoint(int seg, float dx, float dy);
    void groupNudgeX(float dx, float snap);
    void groupNudgeY(float dy, float snapRes);

    float xStep(const juce::ModifierKeys &mods) const;
    float yStep(const juce::ModifierKeys &mods) const;
    float unipolarFactor() const;
    // 0 (no quantize) for fine Shift moves so the fine step isn't rounded away
    float xSnapFor(const juce::ModifierKeys &mods) const;
    float ySnapFor(const juce::ModifierKeys &mods) const;

    void announceCursor();
    std::string nodeAnnouncement(int i) const;
    std::string cpAnnouncement(int seg) const;
    std::string formatValue(float v) const;

    int selectableNodeCount() const;
};

} // namespace Overlays
} // namespace Surge
