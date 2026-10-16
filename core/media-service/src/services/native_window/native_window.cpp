#include "services/native_window/native_window.h"
#include "services/native_window/native_window_p.h"

NativeWindow::NativeWindow() : d_ptr(new NativeWindowPrivate())
{
}

NativeWindow::~NativeWindow() = default;

bool NativeWindow::attach(uint64_t parentHandle, bool show)
{
    return d_ptr->attach(parentHandle, show);
}

bool NativeWindow::setRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    return d_ptr->setRect(x, y, width, height);
}

bool NativeWindow::detach()
{
    return d_ptr->detach();
}

bool NativeWindow::close()
{
    return d_ptr->close();
}

uint64_t NativeWindow::nativeHandle() const
{
    return d_ptr->nativeHandle();
}
