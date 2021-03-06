/**
 * Copyright (C) 2015 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QPen>
#include <QBrush>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QDebug>
#include <QJsonArray>

#include "ddockmenu.h"
#include "dmenucontent.h"
#include "utils.h"

DDockMenu::DDockMenu(DDockMenu *parent):
    DArrowRectangle(DArrowRectangle::ArrowBottom, parent),
    m_menuContent(new DMenuContent(this))
{
    setAccessibleName("DockMenu");
    setBackgroundColor(QColor::fromRgb(18, 18, 18, 255 * 0.9));
    setBorderColor(QColor::fromRgb(255, 255, 255, 255 * 0.1));
    setMargin(0);

    normalStyle = ItemStyle{Qt::transparent,
            Qt::white,
            QColor("#646464"),
            ":/images/check_dark_normal.png",
            ":/images/arrow-dark.png"};
    hoverStyle = ItemStyle{QColor("#2ca7f8"),
            Qt::white,
            QColor("#646464"),
            ":/images/check_dark_hover.png",
            ":/images/arrow-dark.png"};
    inactiveStyle = ItemStyle{Qt::transparent,
            QColor("#646464"),
            QColor("#646464"),
            ":/images/check_dark_inactive.png",
            ":/images/arrow-dark.png"};
}

void DDockMenu::setItemActivity(const QString &, bool)
{

}

void DDockMenu::setItemChecked(const QString &, bool)
{

}

void DDockMenu::setItemText(const QString &, const QString &)
{

}

void DDockMenu::setItems(QJsonArray items)
{
    m_menuContent->setCurrentIndex(-1);
    m_menuContent->clearActions();

    foreach (QJsonValue item, items) {
        QJsonObject itemObj = item.toObject();

        QAction *action = new QAction(m_menuContent);
        QString itemText = itemObj["itemText"].toString().replace("_", QString()).replace(QRegExp("\\([^)]+\\)"), QString());

        action->setText(itemText);
        action->setEnabled(itemObj["isActive"].toBool());
        action->setCheckable(itemObj["isCheckable"].toBool() || Utils::menuItemCheckableFromId(itemObj["itemId"].toString()));
        action->setChecked(itemObj["checked"].toBool());
        action->setProperty("itemId", itemObj["itemId"].toString());
        action->setProperty("itemIcon", itemObj["itemIcon"].toString());
        action->setProperty("itemIconHover", itemObj["itemIconHover"].toString());
        action->setProperty("itemIconInactive", itemObj["itemIconInactive"].toString());
        action->setProperty("itemSubMenu", itemObj["itemSubMenu"].toObject());

        m_menuContent->addAction(action);
    }

    // adjust its size according to its content.
    m_menuContent->setFixedSize(m_menuContent->contentWidth(),
                                m_menuContent->contentHeight());


    setContent(m_menuContent);
    resizeWithContent();
}

DDockMenu *DDockMenu::getRootMenu()
{
    return this;
}

void DDockMenu::showSubMenu(int, int, const QJsonObject &)
{

}

DDockMenu *DDockMenu::menuUnderPoint(const QPoint point)
{
    if (geometry().contains(point)) {
        return this;
    }
    return nullptr;
}

void DDockMenu::grabFocus()
{
    if (m_menuContent) {
        m_menuContent->grabFocus();
    }
}

void DDockMenu::releaseFocus()
{

}

void DDockMenu::destroyAll()
{
    deleteLater();
}
