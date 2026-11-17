#include "WindowFullscreenPolicy.hpp"
#include "../../../debug/log/Logger.hpp"

using namespace Desktop::View;

bool CWindowFullscreenPolicy::allowedOverFullscreen() const {
    return m_allowedOverFullscreen;
}

void CWindowFullscreenPolicy::setAllowedOverFullscreen(bool allowed) {
    // TEMP DIAGNOSTIC: catch-all probe — every policy write is logged with the
    // caller's return address (map with addr2line against the unstripped build).
    diagMark(std::format("[dbg-policy] setAllowedOver this={} allowed={} caller={:p}", static_cast<const void*>(this), sc<int>(allowed), __builtin_return_address(0)));
    m_allowedOverFullscreen = allowed;
}

bool CWindowFullscreenPolicy::effectiveAllowedOverFullscreen(const SFullscreenStackingContext& context) const {
    return context.isFullscreenWindow || context.pinned || m_allowedOverFullscreen || context.groupedWithFullscreen;
}

const SFullscreenRequestSuppression& CWindowFullscreenPolicy::requestSuppression() const {
    return m_requestSuppression;
}

void CWindowFullscreenPolicy::setRequestSuppression(const SFullscreenRequestSuppression& suppression) {
    m_requestSuppression = suppression;
}

const SPendingClientFullscreenRequest& CWindowFullscreenPolicy::pendingClientRequest() const {
    return m_pendingClientRequest;
}

void CWindowFullscreenPolicy::setPendingClientRequest(Fullscreen::eFullscreenMode mode, std::optional<MONITORID> monitor) {
    m_pendingClientRequest.mode = mode;
    if (mode == Fullscreen::FSMODE_FULLSCREEN)
        m_pendingClientRequest.monitor = monitor;
    else
        m_pendingClientRequest.monitor.reset();
}

void CWindowFullscreenPolicy::clearPendingClientMode(Fullscreen::eFullscreenMode mode) {
    if (m_pendingClientRequest.mode != mode)
        return;

    m_pendingClientRequest = {};
}

SPendingClientFullscreenRequest CWindowFullscreenPolicy::consumePendingClientRequest() {
    auto request           = m_pendingClientRequest;
    m_pendingClientRequest = {};
    return request;
}

void CWindowFullscreenPolicy::expectMaximizeEcho() {
    m_expectsMaximizeEcho = true;
}

void CWindowFullscreenPolicy::clearExpectedMaximizeEcho() {
    m_expectsMaximizeEcho = false;
}

bool CWindowFullscreenPolicy::consumeExpectedMaximizeEcho(bool maximized) {
    if (!m_expectsMaximizeEcho)
        return false;

    m_expectsMaximizeEcho = false;
    return maximized;
}

bool CWindowFullscreenPolicy::pinFullscreened() const {
    return m_pinFullscreened;
}

void CWindowFullscreenPolicy::setPinFullscreened(bool pinFullscreened) {
    m_pinFullscreened = pinFullscreened;
}

bool CWindowFullscreenPolicy::restoreClientMaximized() const {
    return m_restoreClientMaximized;
}

void CWindowFullscreenPolicy::setRestoreClientMaximized(bool restore) {
    m_restoreClientMaximized = restore;
}
