/**
 * CWB API (Plain C WebBrowser Control mini API) - Header file
 *
 * Version 1.0, Jan 2022 by Eric M.
 *
 * CWB (C WebBrowser) API is a minimalist implementation of a WebBrowser control
 * adapted to C WinAPI environment. The API allow to embed a WebBrowser
 * control instance (Internet Explorer/Trident) within a common WinAPI window.
 *
 * This mini API is widely inspired from the Codeguru's CWebPage.c example (see
 * link bellow) with help of the René Nyffenegger's C++ implementation (see link
 * bellow) for frame resize handling and navigation interception.
 *
 * Codeguru's Plain C Web Page tutorial :
 *  https://www.codeguru.com/network/display-a-web-page-in-a-plain-c-win32-application/
 *
 * René Nyffenegger's MSHTML Web Browser demonstration :
 *  https://renenyffenegger.ch/notes/Windows/development/MSHTML/index
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the
 * use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *     claim that you wrote the original software. If you use this software in
 *     a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not
 *     be misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 */
#ifndef CWEBCONTROL_H_INCLUDED
#define CWEBCONTROL_H_INCLUDED

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif


/** \brief Attach Web Browser to window
 *
 * Attaches a new Web Browser instance to an existing window.
 *
 * \param[in]  hWnd         Window handle to attach Web Browser instance.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserAttach(HWND hWnd);


/** \brief Create Web Browser window
 *
 * Creates a new Web Browser window or child control.
 *
 * This function first create a new window with the given parameters then
 * attach a new Web Browser instance to it. The created window uses use its own
 * Window Procedure to manage its Web Browser instance.
 *
 * This function is the best way to embed a Web Browser as window child
 * control by specifying a parent window handle and WS_CHILD style.
 *
 * \param[in]  dwExStyle    The extended window style of the window being created.
 * \param[in]  dwStyle      The style of the window being created.
 * \param[in]  hWndParent   A handle to the parent or owner window of the window being created.
 * \param[in]  hMenu        A handle to a menu, or specifies a child-window identifier, depending on the window style.
 * \param[in]  hInstance    A handle to a menu, or specifies a child-window identifier, depending on the window style.
 *
 * \return Handle to created window with Web Browser instance or NULL if error.
 */
HWND WebBrowserCreate(DWORD dwExStyle, DWORD dwStyle, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance);


/** \brief Release Web Browser instance
 *
 * Releases a previously attached Web Browser instance from a window.
 *
 * \param[in]  hWnd    Handle to the window with Web Browser instance.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserRelease(HWND hWnd);


/** \brief Write to Web Browser document (DOM)
 *
 * Write the given HTML content into Web Browser HTML document (DOM).
 *
 * \param[in]  hWnd    Handle to the window with Web Browser instance.
 * \param[in]  lpHtml  HTML content to write.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserDocumentWrite(HWND hWnd, LPCWSTR lpHtml);


/** \brief Close Web Browser document (DOM)
 *
 * Closes (reset) the Web Browser HTML document (DOM).
 *
 * \param[in]  hWnd    Handle to the window with Web Browser instance.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserDocumentClose(HWND hWnd);


/** \brief Open location
 *
 * Orders the specified instance to open the location
 * at given URL.
 *
 * \param[in]  hWnd     Handle to the window with Web Browser instance.
 * \param[in]  lpUrl    URL to open.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserNavigate(HWND hWnd, LPCWSTR lpUrl);


/** \brief Resize Web Browser frame
 *
 * Resizes the Web Browser frame.
 *
 * \param[in]  hWnd     Handle to the window with Web Browser instance.
 * \param[in]  w        The new width of the frame, in pixels.
 * \param[in]  h        The new height of the frame, in pixels.
 *
 * \return Zero if succeed, non-zero value otherwise.
 */
long WebBrowserResize(HWND hWnd, int w, int h);

/** \brief Before Navigate callback
 *
 * Definition for callback function called before browser navigate to URL.
 *
 * \param[in]  lpUrl    Navigation target URL.
 * \param[out] pCancel  Pointer to cancel value to set.
 *
 * \return Zero or non-zero value to cancel or allow navigation.
 */
typedef void (*CwbBeforeNav)(LPCWSTR lpUrl, SHORT* pCancel);

/** \brief Set Before Navigation callback
 *
 * Defines the Before Navigation callback function the specified instance
 *
 * \param[in]  hWnd         Handle to the window with Web Browser instance.
 * \param[in]  pBeforeNav   Pointer to Before Navigate callback function.
 *
 * \return The previously assigned function pointer.
 */
CwbBeforeNav WebBrowserSetBeforeNav(HWND hWnd, CwbBeforeNav pBeforeNav);

#ifdef __cplusplus
}
#endif

#endif // CWEBCONTROL_H_INCLUDED
