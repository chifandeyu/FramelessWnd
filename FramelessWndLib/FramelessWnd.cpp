
/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

/* 																										 */
/* 																										 */
/* File is originally from https://github.com/qtproject/qt-solutions/tree/master/qtwinmigrate/src        */
/* 																										 */
/* It has been modified to support borderless window (HTTRANSPARENT) & to remove pre Qt5 cruft          */
/* 																										 */
/* 																										 */

#include "FramelessWnd.h"
#include "WinNativeWindow.h"
#include <QLabel>
#include <QApplication>
#include <QEvent>
#include <QFocusEvent>
#include <qt_windows.h>
#include <QWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QPushButton>
/*!
    \class FramelessWnd FramelessWnd.h
    \brief The FramelessWnd class is a Qt widget that can be child of a
    native Win32 widget.

    The FramelessWnd class is the bridge between an existing application
    user interface developed using native Win32 APIs or toolkits like
    MFC, and Qt based GUI elements.

    Using FramelessWnd as the parent of QDialogs will ensure that
    modality, placement and stacking works properly throughout the
    entire application. If the child widget is a top level window that
    uses the \c WDestructiveClose flag, FramelessWnd will destroy itself
    when the child window closes down.

    Applications moving to Qt can use FramelessWnd to add new
    functionality, and gradually replace the existing interface.
*/

FramelessWnd::FramelessWnd(QString name)
    : QWidget(nullptr),
      winLayout(nullptr),
      centralwidget(nullptr),
      m_ParentNativeWindowHandle(nullptr),
      _prevFocus(nullptr),
      _reenableParent(false)
{
    initUI(name);
}

FramelessWnd::FramelessWnd(QWidget *contentWidget, QString name)
    : centralwidget(contentWidget),
    QWidget(nullptr),
    winLayout(nullptr),
    m_ParentNativeWindowHandle(nullptr),
    _prevFocus(nullptr),
    _reenableParent(false)
{
    initUI(name);
}

void FramelessWnd::setupUI()
{
    //Windows example of adding a toolbar + min/max/close buttons

    //Add the titleBar
    titleBar = new QWidget(this);
    titleBar->setObjectName("titleBar");
    if(!centralwidget){
        centralwidget = new QWidget(this);
        centralwidget->setObjectName("centralwidget");
    }

    //Create the min/max/close buttons
    minimizeBtn = new QPushButton();
    minimizeBtn->setObjectName("minimizeButton");
    maximizeBtn = new QPushButton();
    maximizeBtn->setObjectName("maximizeButton");
    closeBtn = new QPushButton();
    closeBtn->setObjectName("closeButton");

    titleHLayout = new QHBoxLayout(titleBar);
    titleHLayout->setSpacing(0);
    titleHLayout->setMargin(0);

    //Create a title label just because
    titleLabel = new QLabel();
    titleLabel->setObjectName("titleLabel");
    //	titleLabel->setFixedWidth(160);
    titleLabel->setStyleSheet("color: #ffffff;");
    //Set it transparent to mouse events such that you can click and drag when moused over the label
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    //Add stretch & title label
    titleHLayout->addStretch();
    titleHLayout->addWidget(titleLabel);
    titleHLayout->addStretch();

    maximizeBtn->setCheckable(true);

    titleHLayout->addWidget(minimizeBtn);
    titleHLayout->addWidget(maximizeBtn);
    titleHLayout->addWidget(closeBtn);
    //An actual app should use icons for the buttons instead of text
    //and style the different button states / widget margins in css
}

void FramelessWnd::initUI(const QString &name)
{
    //Create a native window and give it geometry values * devicePixelRatio for HiDPI support
    p_ParentWinNativeWindow = new WinNativeWindow(1  * window()->devicePixelRatio()
        , 1 * window()->devicePixelRatio()
        , 1 * window()->devicePixelRatio()
        , 1 * window()->devicePixelRatio()
        , name.isEmpty() ? QString(QApplication::applicationName()) : name);

    //If you want to set a minimize size for your app, do so here
    //p_ParentWinNativeWindow->setMinimumSize(1024 * window()->devicePixelRatio(), 768 * window()->devicePixelRatio());

    //If you want to set a maximum size for your app, do so here
    //p_ParentWinNativeWindow->setMaximumSize(1024 * window()->devicePixelRatio(), 768 * window()->devicePixelRatio());

    //Save the native window handle for shorthand use
    m_ParentNativeWindowHandle = p_ParentWinNativeWindow->hWnd;
    Q_ASSERT(m_ParentNativeWindowHandle);

    //Create the child window & embed it into the native one
    if (m_ParentNativeWindowHandle)
    {
        setWindowFlags(Qt::FramelessWindowHint);
        setProperty("_q_embedded_native_parent_handle", (WId)m_ParentNativeWindowHandle);
        SetWindowLong((HWND)winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

        SetParent((HWND)winId(), m_ParentNativeWindowHandle);
        QEvent e(QEvent::EmbeddingControl);
        QApplication::sendEvent(this, &e);
    }

    //Pass along our window handle & widget pointer to WinFramelessWidget so we can exchange messages
    p_ParentWinNativeWindow->childWindow = (HWND)winId();
    p_ParentWinNativeWindow->childWidget = this;

    //Clear margins & spacing & add the layout to prepare for the MainAppWidget
    setContentsMargins(0, 0, 0, 0);
    winLayout = new QVBoxLayout;
    winLayout->setContentsMargins(0, 0, 0, 0);
    winLayout->setSpacing(0);
    setLayout(winLayout);

    setupUI();
    //Update the BORDERWIDTH value if needed for HiDPI displays
    BORDERWIDTH = BORDERWIDTH * window()->devicePixelRatio();

    //Update the TOOLBARHEIGHT value to match the height of toolBar * if needed, the HiDPI display
    if (titleBar)
    {
        titleBar->setVisible(true);
        winLayout->addWidget(titleBar);
        TOOLBARHEIGHT = titleBar->height() * window()->devicePixelRatio();
    }
    if (centralwidget) {
        winLayout->addWidget(centralwidget);
    }
    //You need to keep the native window in sync with the Qt window & children, so wire min/max/close buttons to
    //slots inside of FramelessWnd. FramelessWnd can then talk with the native window as needed
    if (minimizeBtn)
    {
        connect(minimizeBtn, &QPushButton::clicked, this, &FramelessWnd::onMinimizeButtonClicked);
    }
    if (maximizeBtn)
    {
        connect(maximizeBtn, &QPushButton::clicked, this, &FramelessWnd::onMaximizeButtonClicked);
    }
    if (closeBtn)
    {
        connect(closeBtn, &QPushButton::clicked, this, &FramelessWnd::onCloseButtonClicked);
    }

    //Send the parent native window a WM_SIZE message to update the widget size
    SendMessage(m_ParentNativeWindowHandle, WM_SIZE, 0, 0);

    //init window size and pos
    QDesktopWidget *desktop = QApplication::desktop();
    int curMonitor = desktop->screenNumber(this);
    QRect availableRc = desktop->availableGeometry(curMonitor);
    int windowXPos, windowYPos, windowWidth, windowHeight;
    windowWidth = 900;
    windowHeight = 600;
    windowXPos = availableRc.x() + (availableRc.width() - windowWidth) / 2;
    windowYPos = availableRc.y() + (availableRc.height() - windowHeight) / 2;
    this->setGeometry(windowXPos, windowYPos, windowWidth, windowHeight);
}

/*!
    Destroys this object, freeing all allocated resources.
*/
FramelessWnd::~FramelessWnd()
{

}

void FramelessWnd::setTitle(const QString& title)
{
    titleLabel->setText(title);
    if (m_ParentNativeWindowHandle) {
        LPCWSTR _cn = (LPCWSTR)title.utf16();
        SetWindowTextW(m_ParentNativeWindowHandle, _cn);
    }
}

void FramelessWnd::setTitleStyle(const QString &qss)
{
    if (titleBar) {
        titleBar->setStyleSheet(qss);
    }
}

/*!
    Returns the handle of the native Win32 parent window.
*/
HWND FramelessWnd::getParentWindow() const
{
    return m_ParentNativeWindowHandle;
}

QHBoxLayout* FramelessWnd::getTitleLayout() const
{
    return titleHLayout;
}

QWidget *FramelessWnd::getCentralwidget() const
{
    return centralwidget;
}

QPushButton* FramelessWnd::getMaximizeButton() const
{
    return maximizeBtn;
}

QPushButton* FramelessWnd::getMinimizeButton() const
{
    return minimizeBtn;
}

QPushButton* FramelessWnd::getCloseButton() const
{
    return closeBtn;
}

/*!
    \reimp
*/
void FramelessWnd::childEvent(QChildEvent *e)
{
    QObject *obj = e->child();
    if (obj->isWidgetType()) {
        if (e->added()) {
        if (obj->isWidgetType()) {
            obj->installEventFilter(this);
        }
        } else if (e->removed() && _reenableParent) {
        _reenableParent = false;
        EnableWindow(m_ParentNativeWindowHandle, true);
            obj->removeEventFilter(this);
        }
    }
    QWidget::childEvent(e);
}

/*! \internal */
void FramelessWnd::saveFocus()
{
    if (!_prevFocus)
        _prevFocus = ::GetFocus();
    if (!_prevFocus)
        _prevFocus = getParentWindow();
}

/*!
    Shows this widget. Overrides QWidget::show().

    \sa showCentered()
*/
void FramelessWnd::show()
{
    ShowWindow(m_ParentNativeWindowHandle, true);
    saveFocus();
    QWidget::show();
}

/*!
    Centers this widget over the native parent window. Use this
    function to have Qt toplevel windows (i.e. dialogs) positioned
    correctly over their native parent windows.

    \code
    FramelessWnd qwin(hParent);
    qwin.center();

    QMessageBox::information(&qwin, "Caption", "Information Text");
    \endcode

    This will center the message box over the client area of hParent.
*/
void FramelessWnd::center()
{
    const QWidget *child = findChild<QWidget*>();
    if (child && !child->isWindow()) {
        qWarning("FramelessWnd::center: Call this function only for FramelessWnd with toplevel children");
    }
    RECT r;
    GetWindowRect(m_ParentNativeWindowHandle, &r);
    setGeometry((r.right-r.left)/2+r.left, (r.bottom-r.top)/2+r.top,0,0);
}

/*!
    \obsolete

    Call center() instead.
*/
void FramelessWnd::showCentered()
{
    center();
    show();
}

void FramelessWnd::setGeometry(int x, int y, int w, int h)
{
    p_ParentWinNativeWindow->setGeometry(x * window()->devicePixelRatio()
        , y * window()->devicePixelRatio()
        , w * window()->devicePixelRatio()
        , h * window()->devicePixelRatio());
}

/*!
    Sets the focus to the window that had the focus before this widget
    was shown, or if there was no previous window, sets the focus to
    the parent window.
*/
void FramelessWnd::resetFocus()
{
    if (_prevFocus)
        ::SetFocus(_prevFocus);
    else
        ::SetFocus(getParentWindow());
}

//Tell the parent native window to minimize
void FramelessWnd::onMinimizeButtonClicked()
{
    SendMessage(m_ParentNativeWindowHandle, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

//Tell the parent native window to maximize or restore as appropriate
void FramelessWnd::onMaximizeButtonClicked()
{
    if (maximizeBtn->isChecked())
    {
        SendMessage(m_ParentNativeWindowHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }
    else
    {
        SendMessage(m_ParentNativeWindowHandle, WM_SYSCOMMAND, SC_RESTORE, 0);
    }
}

void FramelessWnd::onCloseButtonClicked()
{
    if(true /* put your check for it if it safe to close your app here */) //eg, does the user need to save a document
    {
        //Safe to close, so hide the parent window
        ShowWindow(m_ParentNativeWindowHandle, false);

        //And then quit
        QApplication::quit();
    }
    else
    {
        //Do nothing, and thus, don't actually close the window
    }
}

bool FramelessWnd::nativeEvent(const QByteArray &, void *message, long *result)
{
    MSG *msg = (MSG *)message;

    if (msg->message == WM_SETFOCUS)
    {
        Qt::FocusReason reason;
        if (::GetKeyState(VK_LBUTTON) < 0 || ::GetKeyState(VK_RBUTTON) < 0)
            reason = Qt::MouseFocusReason;
        else if (::GetKeyState(VK_SHIFT) < 0)
            reason = Qt::BacktabFocusReason;
        else
            reason = Qt::TabFocusReason;
        QFocusEvent e(QEvent::FocusIn, reason);
        QApplication::sendEvent(this, &e);
    }

    //Only close if safeToClose clears()
    if (msg->message == WM_CLOSE)
    {
        if (true /* put your check for it if it safe to close your app here */) //eg, does the user need to save a document
        {
            //Safe to close, so hide the parent window
            ShowWindow(m_ParentNativeWindowHandle, false);

            //And then quit
            QApplication::quit();
        }
        else
        {
            *result = 0; //Set the message to 0 to ignore it, and thus, don't actually close
            return true;
        }
    }

    //Double check WM_SIZE messages to see if the parent native window is maximized
    if (msg->message == WM_SIZE)
    {
        if (maximizeBtn)
        {
            //Get the window state
            WINDOWPLACEMENT wp;
            GetWindowPlacement(m_ParentNativeWindowHandle, &wp);

            //If we're maximized,
            if (wp.showCmd == SW_MAXIMIZE)
            {
                //Maximize button should show as Restore
                maximizeBtn->setChecked(true);
            }
            else
            {
                //Maximize button should show as Maximize
                maximizeBtn->setChecked(false);
            }
        }
    }

    //Pass NCHITTESTS on the window edges as determined by BORDERWIDTH & TOOLBARHEIGHT through to the parent native window
    if (msg->message == WM_NCHITTEST)
    {
        RECT WindowRect;
        int x, y;

        GetWindowRect(msg->hwnd, &WindowRect);
        x = GET_X_LPARAM(msg->lParam) - WindowRect.left;
        y = GET_Y_LPARAM(msg->lParam) - WindowRect.top;

        if (x >= BORDERWIDTH && x <= WindowRect.right - WindowRect.left - BORDERWIDTH && y >= BORDERWIDTH && y <= TOOLBARHEIGHT)
        {
            if (titleBar)
            {
                //If the mouse is over top of the toolbar area BUT is actually positioned over a child widget of the toolbar,
                //Then we don't want to enable dragging. This allows for buttons in the toolbar, eg, a Maximize button, to keep the mouse interaction
                if (QApplication::widgetAt(QCursor::pos()) != titleBar)
                    return false;
            }

            //The mouse is over the toolbar area & is NOT over a child of the toolbar, so pass this message
            //through to the native window for HTCAPTION dragging
            *result = HTTRANSPARENT;
            return true;

        }
        else if (x < BORDERWIDTH && y < BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x > WindowRect.right - WindowRect.left - BORDERWIDTH && y < BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x > WindowRect.right - WindowRect.left - BORDERWIDTH && y > WindowRect.bottom - WindowRect.top - BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x < BORDERWIDTH && y > WindowRect.bottom - WindowRect.top - BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x < BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (y < BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (x > WindowRect.right - WindowRect.left - BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }
        else if (y > WindowRect.bottom - WindowRect.top - BORDERWIDTH)
        {
            *result = HTTRANSPARENT;
            return true;
        }

        return false;
    }

    return false;
}

/*!
    \reimp
*/
bool FramelessWnd::eventFilter(QObject *o, QEvent *e)
{
    QWidget *w = (QWidget*)o;

    switch (e->type())
    {
        case QEvent::WindowDeactivate:
        if (w->isModal() && w->isHidden())
            BringWindowToTop(m_ParentNativeWindowHandle);
        break;

        case QEvent::Hide:
        if (_reenableParent) {
            EnableWindow(m_ParentNativeWindowHandle, true);
            _reenableParent = false;
        }
        resetFocus();

        if (w->testAttribute(Qt::WA_DeleteOnClose) && w->isWindow())
            deleteLater();
        break;

        case QEvent::Show:
        if (w->isWindow()) {
            saveFocus();
            hide();
            if (w->isModal() && !_reenableParent) {
            EnableWindow(m_ParentNativeWindowHandle, false);
            _reenableParent = true;
            }
        }
        break;

        case QEvent::Close:
        {
            ::SetActiveWindow(m_ParentNativeWindowHandle);
            if (w->testAttribute(Qt::WA_DeleteOnClose))
                deleteLater();
            break;
        }
        default:
        break;
    }

    return QWidget::eventFilter(o, e);
}

/*! \reimp
*/
void FramelessWnd::focusInEvent(QFocusEvent *e)
{
    QWidget *candidate = this;

    switch (e->reason()) {
    case Qt::TabFocusReason:
    case Qt::BacktabFocusReason:
        while (!(candidate->focusPolicy() & Qt::TabFocus)) {
            candidate = candidate->nextInFocusChain();
            if (candidate == this) {
                candidate = 0;
                break;
            }
        }
        if (candidate) {
            candidate->setFocus(e->reason());
            if (e->reason() == Qt::BacktabFocusReason || e->reason() == Qt::TabFocusReason) {
                candidate->setAttribute(Qt::WA_KeyboardFocusChange);
                candidate->window()->setAttribute(Qt::WA_KeyboardFocusChange);
            }
            if (e->reason() == Qt::BacktabFocusReason)
                QWidget::focusNextPrevChild(false);
        }
        break;
    default:
        break;
    }
}

/*! \reimp
*/
bool FramelessWnd::focusNextPrevChild(bool next)
{
    QWidget *curFocus = focusWidget();
    if (!next) {
        if (!curFocus->isWindow()) {
            QWidget *nextFocus = curFocus->nextInFocusChain();
            QWidget *prevFocus = 0;
            QWidget *topLevel = 0;
            while (nextFocus != curFocus) {
                if (nextFocus->focusPolicy() & Qt::TabFocus) {
                    prevFocus = nextFocus;
                    topLevel = 0;
                }
                nextFocus = nextFocus->nextInFocusChain();
            }

            if (!topLevel) {
                return QWidget::focusNextPrevChild(false);
            }
        }
    } else {
        QWidget *nextFocus = curFocus;
        while (1 && nextFocus != 0) {
            nextFocus = nextFocus->nextInFocusChain();
            if (nextFocus->focusPolicy() & Qt::TabFocus) {
                return QWidget::focusNextPrevChild(true);
            }
        }
    }

    ::SetFocus(m_ParentNativeWindowHandle);

    return true;
}
