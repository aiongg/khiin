#pragma once

#include "common.h"

namespace khiin::proto {
class Preedit;
class CandidateList;
enum EditState : int;
} // namespace khiin::proto

namespace khiin::engine {

enum class InputMode;
class Engine;

/**
 * The BufferMgr represents one user-composition session, from
 * first input until the composition is committed. It maintains
 * the state of character input, caret position, and conversion
 * candidate search results. All information necessary for display
 * in the application is provided through the Preedit and CandidateList
 * protobuf messages created in this class.
 */
class BufferMgr {
  public:
    static BufferMgr *Create(Engine *engine);

    /**
     * Builds a Preedit protobuf message, which may be relayed to
     * the end user application. Preedit contains all of the information
     * necessary to render the composition window.
     */
    virtual void BuildPreedit(proto::Preedit *preedit) = 0;

    /**
     * Builds a CandidateList protobuf message for the end user application.
     * Candidate display (e.g. pagination) is to be decided by the
     * application. In order to keep the Preedit in sync with the user's
     * conversin choices, the application must relay the focused or
     * selected candidate's ID back to the Engine.
     */
    virtual void GetCandidates(proto::CandidateList *candidate_list) = 0;

    /**
     * Returns the current EditState
     */
    virtual proto::EditState edit_state() = 0;

    /**
     * Returns |true| if the buffer is empty, e.g. when:
     * - User has not yet begun typing
     * - User has committed a composition
     * - User has cancelled a composition
     */
    virtual bool IsEmpty() = 0;

    /**
     * Reset the buffer back to its original state
     */
    virtual void Clear() = 0;

    /**
     * Insert one ascii character into the raw buffer. If possible,
     * the composition will be displayed as Lomaji directly.
     *
     * For example, typing |a2| will display |á|, which the user may
     * either commit directly or convert. Conversion candidates are
     * based on the raw key inputs, and may therefore be different from
     * what is currently displayed in the composition.
     */
    virtual void Insert(char ch) = 0;

    /**
     * Erase one logical character in |direction| from the caret position
     * Logical characters include e.g. tone marks and diacritics
     */
    virtual void Erase(CursorDirection direction) = 0;

    /**
     * - When composing or navigating a converted segment by character,
     *   moves the caret one logical character at a time.
     * - When navigating by segment, moves the segment focus.
     * - When navigating candidates, may either change candidate focus
     *   or segment focus, depending on the state and settings.
     */
    virtual void HandleLeftRight(CursorDirection direction) = 0;

    /**
     * Returns |true| if the composition should be committed (during
     * composition or after conversion). Returns |false| otherwise
     * (during candidate selection). In the latter case, it is
     * equivalent to calling |SelectCandidate| with index 0.
     *
     * To select while composing, use |HandleSelectOrFocus| instead.
     *
     * This method should typically be called with ENTER.
     */
    virtual bool HandleSelectOrCommit() = 0;

    /**
     * When composing, selects the first conversion candidate.
     * After conversion, focuses the next candidate.
     * To select the focused conversion, call |HandleSelectOrCommit|.
     *
     * This method should typically be called with SPACEBAR.
     */
    virtual void HandleSelectOrFocus() = 0;

    /**
     * Swaps the focused composition element with the specified candidate.
     * Candidate indices refer to those provided by |GetCandidates|.
     * This method does not change the candidate list.
     */
    virtual void FocusCandidate(size_t index) = 0;

    /**
     * Same as |FocusCandidate|, where |index| is the current index
     * incremented by one, wrapping from the end back to 0.
     */
    virtual void FocusNextCandidate() = 0;

    /**
     * Same as |FocusCandidate|, where |index| is the current index
     * decremented by one, wrapping from 0 back to the end.
     */
    virtual void FocusPrevCandidate() = 0;

    /**
     * Internally calls |FocusCandidate|, and then moves the
     * composition focus either to the start (if previously composing)
     * or to the end of the selection (if already converted). The
     * candidate list will be hidden after calling this method and
     * must be updated before display.
     */
    virtual void SelectCandidate(size_t id) = 0;
};

} // namespace khiin::engine
