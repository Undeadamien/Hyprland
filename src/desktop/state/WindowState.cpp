#include "WindowState.hpp"
#include "../../event/EventBus.hpp"
#include "../../render/Renderer.hpp"
#include "../view/window/Window.hpp"
#include "../view/window/WindowFullscreenPolicy.hpp"
#include "../view/window/WindowPresentation.hpp"

#include <algorithm>
#include <ranges>

using namespace Desktop;

CWindowState::CWindowState() {
    m_listeners.viewCreate = Event::bus()->m_events.view.create.listen([this](PHLVIEW view) {
        const auto WINDOW = View::CWindow::fromView(view);
        if (!WINDOW)
            return;

        m_windows.emplace_back(WINDOW);
    });
}

void CWindowState::removeSafe(PHLWINDOW w) {
    std::erase_if(m_windows, [&w](auto& el) { return !el || el == w; });
}

const std::vector<PHLWINDOW>& CWindowState::windows() const {
    return m_windows;
}

CWindowQuery CWindowState::query() const {
    return CWindowQuery{*this};
}

bool CWindowState::groupsLocked() const {
    return m_groupsLocked;
}

void CWindowState::setGroupsLocked(bool locked) {
    m_groupsLocked = locked;
}

void CWindowState::raise(PHLWINDOW w) {
    moveToZ(w, true);
}

void CWindowState::lower(PHLWINDOW w) {
    moveToZ(w, false);
}

bool CWindowState::isAlwaysOnTop(PHLWINDOW w) {
    return w && (w->m_state & View::WINDOW_STATE_ALWAYS_ON_TOP);
}

std::vector<PHLWINDOW>::iterator CWindowState::firstAlwaysOnTop() {
    return std::ranges::find_if(m_windows, [](PHLWINDOW w) { return CWindowState::isAlwaysOnTop(w); });
}

std::vector<PHLWINDOW>::const_iterator CWindowState::firstAlwaysOnTop() const {
    return std::ranges::find_if(m_windows, [](PHLWINDOW w) { return CWindowState::isAlwaysOnTop(w); });
}

void CWindowState::moveWindowToPartition(PHLWINDOW w, bool top) {
    if (!w || m_windows.empty())
        return;

    auto it = std::ranges::find(m_windows, w);
    if (it == m_windows.end())
        return;

    const bool wIsAOT = isAlwaysOnTop(w);
    m_windows.erase(it);

    auto aotBegin = firstAlwaysOnTop();
    std::vector<PHLWINDOW>::iterator insertPos;

    if (top) {
        if (wIsAOT) {
            insertPos = m_windows.end();
        } else {
            insertPos = (aotBegin != m_windows.end()) ? aotBegin : m_windows.end();
        }
    } else {
        if (wIsAOT) {
            insertPos = (aotBegin != m_windows.end()) ? aotBegin : m_windows.end();
        } else {
            insertPos = m_windows.begin();
        }
    }

    m_windows.insert(insertPos, w);

    if (w->mapped())
        g_pHyprRenderer->damageMonitor(w->m_monitor.lock());
}

void CWindowState::moveToZ(PHLWINDOW w, bool top) {
    if (!View::validMapped(w) || m_windows.empty())
        return;

    w->fullscreenPolicy().setAllowedOverFullscreen(top);
    w->updateFullscreenInputState();
    *w->presentation().alpha(View::WINDOW_ALPHA_FULLSCREEN) = w->isBlockedByFullscreen() ? 0.F : 1.F;

    const bool wIsAOT = isAlwaysOnTop(w);
    const auto aotBegin = firstAlwaysOnTop();
    const bool aotExists = aotBegin != m_windows.end();

    // Check if already at correct boundary
    if (top) {
        if (wIsAOT) {
            if (w == m_windows.back())
                return;
        } else {
            if (aotExists && w == std::prev(aotBegin))
                return;
            if (!aotExists && w == m_windows.back())
                return;
        }
    } else {
        if (wIsAOT) {
            if (aotExists && w == *aotBegin)
                return;
        } else {
            if (w == m_windows.front())
                return;
        }
    }

    if (!w->backend().isX11()) {
        moveWindowToPartition(w, top);
        return;
    }

    // X11 transient stack
    std::vector<PHLWINDOW> toMove;

    auto collectX11Stack = [&](PHLWINDOW pw, auto&& collectX11Stack) -> void {
        if (top)
            toMove.emplace_back(pw);
        else
            toMove.insert(toMove.begin(), pw);

        for (auto const& other : m_windows) {
            if (other->mapped() && !other->isHidden() && other->backend().isX11() && other->backend().parent() == pw && other != pw &&
                std::ranges::find(toMove, other) == toMove.end())
                collectX11Stack(other, collectX11Stack);
        }
    };

    collectX11Stack(w, collectX11Stack);
    for (auto const& pw : toMove) {
        moveWindowToPartition(pw, top);
    }
}

void CWindowState::moveToTop(PHLWINDOW w) {
    moveWindowToPartition(w, true);
}

void CWindowState::moveToBottom(PHLWINDOW w) {
    moveWindowToPartition(w, false);
}

void CWindowState::clear() {
    m_windows.clear();
}

UP<CWindowState>& Desktop::windowState() {
    static UP<CWindowState> state = makeUnique<CWindowState>();
    return state;
}
