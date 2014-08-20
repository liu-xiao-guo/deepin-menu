#include <QMargins>
#include <QtGlobal>
#include <QGraphicsDropShadowEffect>
#include <QJsonArray>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QSharedPointer>
#include <QRegExp>
#include <QPoint>
#include <QDebug>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include "dmenubase.h"
#include "dmenucontent.h"
#include "utils.h"

DMenuBase::DMenuBase(QWidget *parent) :
    QWidget(parent, Qt::Tool | Qt::BypassWindowManagerHint),
    _subMenu(NULL),
    _radius(4),
    _shadowMargins(QMargins(0, 0, 0, 0))
{
    this->setAttribute(Qt::WA_TranslucentBackground);

    _dropShadow = new QGraphicsDropShadowEffect(this);
    _dropShadow->setBlurRadius(0);
    _dropShadow->setColor(Qt::black);
    _dropShadow->setOffset(0.0);
    this->setGraphicsEffect(_dropShadow);
}

// getters and setters
int DMenuBase::radius()
{
    return _radius;
}

void DMenuBase::setRadius(int radius)
{
    if (_radius != radius) {
        _radius = radius;

        emit radiusChanged(radius);
    }
}

QMargins DMenuBase::shadowMargins()
{
    return _shadowMargins;
}

void DMenuBase::setShadowMargins(QMargins shadowMargins)
{
    if (_shadowMargins != shadowMargins) {
        _shadowMargins = shadowMargins;
        _dropShadow->setBlurRadius(qMax(qMax(_shadowMargins.left(), _shadowMargins.top()),
                                        qMax(_shadowMargins.right(), _shadowMargins.bottom())));

        emit shadowMarginsChanged(shadowMargins);
    }
}

QMargins DMenuBase::menuContentMargins()
{
    return _menuContentMargins;
}

void DMenuBase::setMenuContentMargins(QMargins margins)
{
    if(_menuContentMargins != margins) {
        _menuContentMargins = margins;

        emit menuContentMarginsChanged(margins);
    }
}

int DMenuBase::itemLeftSpacing()
{
    return _itemLeftSpacing;
}

void DMenuBase::setItemLeftSpacing(int spacing)
{
    if (_itemLeftSpacing != spacing) {
        _itemLeftSpacing = spacing;

        emit itemLeftSpacingChanged(spacing);
    }
}

int DMenuBase::itemCenterSpacing()
{
    return _itemCenterSpacing;
}

void DMenuBase::setItemCenterSpacing(int spacing)
{
    if (_itemCenterSpacing != spacing) {
        _itemCenterSpacing = spacing;

        emit itemCenterSpacingChanged(spacing);
    }
}

int DMenuBase::itemRightSpacing()
{
    return _itemRightSpacing;
}

void DMenuBase::setItemRightSpacing(int spacing)
{
    if (_itemRightSpacing != spacing) {
        _itemRightSpacing = spacing;

        emit itemRightSpacingChanged(spacing);
    }
}

QSharedPointer<DMenuContent> DMenuBase::menuContent()
{
    return _menuContent;
}

void DMenuBase::setMenuContent(QSharedPointer<DMenuContent> content)
{
    _menuContent = content;
    _menuContent->setContentsMargins(_menuContentMargins);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(QMargins(0, _menuContentMargins.top(), 0, _menuContentMargins.bottom()));
    layout->addWidget(_menuContent.data());

    this->setLayout(layout);
}

DMenuBase* DMenuBase::subMenu()
{
    return _subMenu;
}

void DMenuBase::setContent(QJsonArray items)
{
    Q_ASSERT(this->menuContent());
    this->menuContent()->setCurrentIndex(-1);
    this->menuContent()->clearActions();

    foreach (QJsonValue item, items) {
        QJsonObject itemObj = item.toObject();

        QAction *action = new QAction(this->menuContent().data());
        QRegExp regexp("_(.)");
        regexp.indexIn(itemObj["itemText"].toString());
        QString navKey = regexp.cap(1);
        QString itemText = itemObj["itemText"].toString().replace(regexp, navKey);

        action->setText(itemText);
        action->setEnabled(itemObj["isActive"].toBool());
        action->setCheckable(Utils::menuItemCheckableFromId(itemObj["itemId"].toString()));
        action->setChecked(itemObj["checked"].toBool());
        action->setProperty("itemId", itemObj["itemId"].toString());
        action->setProperty("itemIcon", itemObj["itemIcon"].toString());
        action->setProperty("itemIconHover", itemObj["itemIconHover"].toString());
        action->setProperty("itemIconInactive", itemObj["itemIconInactive"].toString());
        action->setProperty("itemSubMenu", itemObj["itemSubMenu"].toObject());
        action->setProperty("itemNavKey", navKey);

        _menuContent->addAction(action);
    }

    // adjust its size according to its content.
    this->resize(_menuContent->contentWidth()
                 + _shadowMargins.left()
                 + _shadowMargins.right()
                 + _menuContentMargins.left()
                 + _menuContentMargins.right(),
                 _menuContent->contentHeight()
                 + _shadowMargins.top()
                 + _shadowMargins.bottom()
                 + _menuContentMargins.top()
                 + _menuContentMargins.bottom()
                 + this->contentsMargins().bottom() - _shadowMargins.bottom());
}

void DMenuBase::destroyAll()
{
    if(this->parent()) {
        DMenuBase *parent = qobject_cast<DMenuBase*>(this->parent());
        Q_ASSERT(parent);
        parent->destroyAll();
    } else {
        this->deleteLater();
    }
}

void DMenuBase::grabFocus()
{
    this->menuContent()->grabKeyboard();
    this->menuContent()->grabMouse();
}

DMenuBase *DMenuBase::menuUnderPoint(QPoint point)
{
    if(this->parent()) {
        DMenuBase *parent = qobject_cast<DMenuBase*>(this->parent());
        Q_ASSERT(parent);

        return parent->menuUnderPoint(point);
    } else {
        DMenuBase *subMenu =this;
        while (subMenu) {
            if (Utils::pointInRect(point, subMenu->geometry())) {
                return subMenu;
            }
            subMenu = subMenu->subMenu();
        }
        return NULL;
    }
}

void DMenuBase::setItemActivity(const QString &itemId, bool isActive)
{
    QString prop("%1Activity");
    this->setProperty(prop.arg(itemId).toLatin1(), isActive);
}

void DMenuBase::setItemChecked(const QString &itemId, bool checked)
{
    QString prop("%1Checked");
    this->setProperty(prop.arg(itemId).toLatin1(), checked);
}

void DMenuBase::setItemText(const QString &itemId, const QString &text)
{
    QString prop("%1Text");
    this->setProperty(prop.arg(itemId).toLatin1(), text);
}

const DMenuBase::ItemStyle DMenuBase::normalStyle()
{
    return _normalStyle;
}

const DMenuBase::ItemStyle DMenuBase::hoverStyle()
{
    return _hoverStyle;
}

const DMenuBase::ItemStyle DMenuBase::inactiveStyle()
{
    return _inactiveStyle;
}

DMenuBase *DMenuBase::getRootMenu()
{
    QObject *root = this;
    while (root) {
        if (root->parent()) {
            root = root->parent();
        } else {
            break;
        }
    }

    DMenuBase *result = qobject_cast<DMenuBase *>(root);
    Q_ASSERT(result);

    return result;
}

// override methods
bool DMenuBase::nativeEvent(const QByteArray &eventType, void *message, long *)
{
    if (eventType=="xcb_generic_event_t") {
        xcb_generic_event_t *event = static_cast<xcb_generic_event_t*>(message);
        const uint8_t responseType = event->response_type & ~0x80;

        if (responseType == XCB_BUTTON_PRESS) {
            xcb_button_press_event_t *ev = reinterpret_cast<xcb_button_press_event_t*>(event);
            qDebug() << "nativeEvent" <<  responseType <<
                        ev->detail << ev->child << ev->root_x << ev->root_y;
            if (!this->menuUnderPoint(QPoint(ev->root_x, ev->root_y))) {
                this->destroyAll();
            } else if (_menuContent){
                _menuContent->doCurrentAction();
            }
        }
    }
    return false;
}
