#include <QDebug>
#include <QQuickView>
#include <QQuickItem>
#include <QQmlEngine>
#include <QQmlContext>
#include <QColor>
#include <QDBusConnection>
#include <QGuiApplication>

#include "utils.h"
#include "menu_object.h"
#include "manager_object.h"

MenuObject::MenuObject(ManagerObject *manager):
    QObject()
{
    this->manager = manager;
    this->menu = new QQuickView();

    connect(Utils::instance(), SIGNAL(itemClicked(QString,bool)), this, SIGNAL(ItemInvoked(QString,bool)));
    connect(Utils::instance(), SIGNAL(menuDisappeared()), manager, SLOT(UnregisterMenu()));
}

MenuObject::~MenuObject()
{
    if(menu) delete menu;
    menu = NULL;
}

void MenuObject::SetItemActivity(const QString &itemId, bool isActive)
{
    QMetaObject::invokeMethod(menu->rootObject(), "setActive", Q_ARG(QString, itemId), Q_ARG(bool, isActive));
}

void MenuObject::SetItemChecked(const QString &itemId, bool checked)
{
    QMetaObject::invokeMethod(menu->rootObject(), "setChecked", Q_ARG(QString, itemId), Q_ARG(bool, checked));
}

void MenuObject::SetItemText(const QString &itemId, const QString &text)
{
    QMetaObject::invokeMethod(menu->rootObject(), "setText", Q_ARG(QString, itemId), Q_ARG(QString, text));
}

void MenuObject::ShowMenu(const QString &menuJsonContent)
{
    menu->setFlags(Qt::Tool | Qt::BypassWindowManagerHint);
    menu->setResizeMode(QQuickView::SizeViewToRootObject);
    menu->setColor(QColor(0, 0, 0, 0));
    menu->setSource(QUrl(QStringLiteral("qrc:///FullscreenBackground.qml")));
    menu->rootContext()->setContextProperty("_utils_", Utils::instance());
    menu->show();
    menu->requestActivate();

    QMetaObject::invokeMethod(menu->rootObject(), "showMenu", Q_ARG(QVariant, menuJsonContent));

    QQmlEngine *engine = menu->rootContext()->engine();
    QObject::connect(engine, SIGNAL(quit()), QGuiApplication::instance(), SLOT(quit()));

    Utils::instance()->grabKeyboard(menu->winId());
}

void MenuObject::destroyMenu()
{
    emit MenuUnregistered();
}