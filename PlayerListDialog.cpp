/*****************************************************************************
* gta5sync GRAND THEFT AUTO V SYNC
* Copyright (C) 2016-2017 Syping
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "PlayerListDialog.h"
#include "ui_PlayerListDialog.h"
#include "AppEnv.h"
#include <QFontMetrics>
#include <QInputDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QDebug>

PlayerListDialog::PlayerListDialog(QStringList players, ProfileDatabase *profileDB, QWidget *parent) :
    QDialog(parent), players(players), profileDB(profileDB),
    ui(new Ui::PlayerListDialog)
{
    // Set Window Flags
    setWindowFlags(windowFlags()^Qt::WindowContextHelpButtonHint);

    listUpdated = false;

    ui->setupUi(this);
    ui->cmdCancel->setDefault(true);
    ui->cmdCancel->setFocus();

    // Set Icon for Apply Button
    if (QIcon::hasThemeIcon("dialog-ok-apply"))
    {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-ok-apply"));
    }
    else if (QIcon::hasThemeIcon("dialog-apply"))
    {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-apply"));
    }
    else if (QIcon::hasThemeIcon("gtk-apply"))
    {
        ui->cmdApply->setIcon(QIcon::fromTheme("gtk-apply"));
    }
    else if (QIcon::hasThemeIcon("dialog-ok"))
    {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-ok"));
    }
    else if (QIcon::hasThemeIcon("gtk-ok"))
    {
        ui->cmdApply->setIcon(QIcon::fromTheme("dialog-ok"));
    }

    // Set Icon for Cancel Button
    if (QIcon::hasThemeIcon("dialog-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("dialog-cancel"));
    }
    else if (QIcon::hasThemeIcon("gtk-cancel"))
    {
        ui->cmdCancel->setIcon(QIcon::fromTheme("gtk-cancel"));
    }

    // Set Icon for Manage Buttons
    if (QIcon::hasThemeIcon("go-previous") && QIcon::hasThemeIcon("go-next") && QIcon::hasThemeIcon("list-add"))
    {
        ui->cmdMakeAv->setIcon(QIcon::fromTheme("go-previous"));
        ui->cmdMakeSe->setIcon(QIcon::fromTheme("go-next"));
        ui->cmdMakeAd->setIcon(QIcon::fromTheme("list-add"));
    }
    else
    {
        drawSwitchButtons();
    }
    buildInterface();

    // DPI calculation
    qreal screenRatio = AppEnv::screenRatio();
    resize(500 * screenRatio, 350 * screenRatio);
}

PlayerListDialog::~PlayerListDialog()
{
    for (QObject *object : ui->listAvPlayers->children())
    {
        delete object;
    }
    for (QObject *object : ui->listSePlayers->children())
    {
        delete object;
    }
    delete ui;
}

void PlayerListDialog::drawSwitchButtons()
{
    QFont painterFont = ui->cmdApply->font();
    QPalette palette;

    QFontMetrics fontMetrics(painterFont);
    QRect makeAvRect = fontMetrics.boundingRect(QRect(0, 0, 0, 0), Qt::AlignCenter | Qt::TextDontClip, "<");
    QRect makeSeRect = fontMetrics.boundingRect(QRect(0, 0, 0, 0), Qt::AlignCenter | Qt::TextDontClip, ">");
    QRect makeAdRect = fontMetrics.boundingRect(QRect(0, 0, 0, 0), Qt::AlignCenter | Qt::TextDontClip, "+");

    int makeAvSize;
    if (makeAvRect.height() > makeAvRect.width())
    {
        makeAvSize = makeAvRect.height();
    }
    else
    {
        makeAvSize = makeAvRect.width();
    }
    int makeSeSize;
    if (makeSeRect.height() > makeSeRect.width())
    {
        makeSeSize = makeSeRect.height();
    }
    else
    {
        makeSeSize = makeSeRect.width();
    }
    int makeAdSize;
    if (makeAdRect.height() > makeAdRect.width())
    {
        makeAdSize = makeAdRect.height();
    }
    else
    {
        makeAdSize = makeAdRect.width();
    }

    QImage avImage(makeAvSize, makeAvSize, QImage::Format_ARGB32_Premultiplied);
    avImage.fill(Qt::transparent);
    QImage seImage(makeSeSize, makeSeSize, QImage::Format_ARGB32_Premultiplied);
    seImage.fill(Qt::transparent);
    QImage adImage(makeAdSize, makeAdSize, QImage::Format_ARGB32_Premultiplied);
    adImage.fill(Qt::transparent);

    QPainter avPainter(&avImage);
    avPainter.setFont(painterFont);
    avPainter.setBrush(palette.buttonText());
    avPainter.drawText(0, 0, makeAvSize, makeAvSize, Qt::AlignCenter | Qt::TextDontClip, "<");
    avPainter.end();
    QPainter sePainter(&seImage);
    sePainter.setFont(painterFont);
    sePainter.setBrush(palette.buttonText());
    sePainter.drawText(0, 0, makeSeSize, makeSeSize, Qt::AlignCenter | Qt::TextDontClip, ">");
    sePainter.end();
    QPainter adPainter(&adImage);
    adPainter.setFont(painterFont);
    adPainter.setBrush(palette.buttonText());
    adPainter.drawText(0, 0, makeAdSize, makeAdSize, Qt::AlignCenter | Qt::TextDontClip, "+");
    adPainter.end();

    ui->cmdMakeAv->setIconSize(avImage.size());
    ui->cmdMakeSe->setIconSize(seImage.size());
    ui->cmdMakeAd->setIconSize(adImage.size());

    ui->cmdMakeAv->setIcon(QIcon(QPixmap::fromImage(avImage)));
    ui->cmdMakeSe->setIcon(QIcon(QPixmap::fromImage(seImage)));
    ui->cmdMakeAd->setIcon(QIcon(QPixmap::fromImage(adImage)));
}

void PlayerListDialog::on_cmdCancel_clicked()
{
    close();
}

void PlayerListDialog::buildInterface()
{
    const QStringList dbPlayers = profileDB->getPlayers();
    for (QString sePlayer : players)
    {
        ui->listSePlayers->addItem(QString("%1 (%2)").arg(sePlayer, profileDB->getPlayerName(sePlayer)));
    }
    for (QString dbPlayer : dbPlayers)
    {
        if (!players.contains(dbPlayer))
        {
            ui->listAvPlayers->addItem(QString("%1 (%2)").arg(dbPlayer, profileDB->getPlayerName(dbPlayer)));
        }
    }
}

void PlayerListDialog::on_cmdMakeAv_clicked()
{
    for (QListWidgetItem *item : ui->listSePlayers->selectedItems())
    {
        QString playerItemText = item->text();
        delete item;
        ui->listAvPlayers->addItem(playerItemText);
        ui->listAvPlayers->sortItems(Qt::AscendingOrder);
    }
}

void PlayerListDialog::on_cmdMakeSe_clicked()
{
    int maxPlayers = 30;
    if (maxPlayers < ui->listSePlayers->count() + ui->listAvPlayers->selectedItems().count())
    {
        QMessageBox::warning(this, tr("Add Players..."), tr("Failed to add more Players because the limit of Players are %1!").arg(QString::number(maxPlayers)));
        return;
    }
    for (QListWidgetItem *item : ui->listAvPlayers->selectedItems())
    {
        QString playerItemText = item->text();
        delete item;
        ui->listSePlayers->addItem(playerItemText);
    }
}

void PlayerListDialog::on_cmdMakeAd_clicked()
{
    bool playerOk;
    int playerID = QInputDialog::getInt(this, tr("Add Player..."), tr("Enter Social Club Player ID"), 1, 1, 214783647, 1, &playerOk, windowFlags());
    if (playerOk)
    {
        for (int i = 0; i < ui->listAvPlayers->count(); ++i)
        {
            QListWidgetItem *item = ui->listAvPlayers->item(i);
            QString playerItemText = item->text();
            if (playerItemText.split(" ").at(0) == QString::number(playerID))
            {
                delete item;
                ui->listSePlayers->addItem(playerItemText);
                return;
            }
        }
        for (int i = 0; i < ui->listSePlayers->count(); ++i)
        {
            QListWidgetItem *item = ui->listSePlayers->item(i);
            QString playerItemText = item->text();
            if (playerItemText.split(" ").at(0) == QString::number(playerID))
            {
                QMessageBox::warning(this, tr("Add Player..."), tr("Failed to add Player %1 because Player %1 is already added!").arg(QString::number(playerID)));
                //ui->listSePlayers->setCurrentItem(item);
                return;
            }
        }
        QString playerItemText = QString("%1 (%1)").arg(QString::number(playerID));
        ui->listSePlayers->addItem(playerItemText);
    }
}

void PlayerListDialog::on_cmdApply_clicked()
{
    players.clear();
    for (int i = 0; i < ui->listSePlayers->count(); ++i)
    {
        players += ui->listSePlayers->item(i)->text().split(" ").at(0);
    }
    emit playerListUpdated(players);
    listUpdated = true;
    close();
}

QStringList PlayerListDialog::getPlayerList() const
{
    return players;
}

bool PlayerListDialog::isListUpdated()
{
    return listUpdated;
}
