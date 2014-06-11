/*
 * Copyright 2014 Daniel Logan
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "distributordialog.h"
#include "ui_distributordialog.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>


#define REFRESH_KEY "refreshIntervalSeconds"
#define SOURCE_DIR_KEY "sourceDir"
#define DEST_DIR_KEY "destDir"


DistributorDialog::DistributorDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DistributorDialog)
{
    ui->setupUi(this);

    QString appData(getenv("APPDATA"));
    dataFileName = appData.append("\\FileDistributor\\settings.dat");
    copier = new Copier();
    copier->moveToThread(&workerThread);
    dirsModel = new QStringListModel(copier->getDestDirs(), this);

    load();
    icon = new QSystemTrayIcon(QIcon(":/res/FD.ico"), this);
    iconMenu = new QMenu(this);
    iconMenu->addAction("Settings", this, SLOT(show()));
    iconMenu->addSeparator();
    iconMenu->addAction("Close", this, SLOT(quit()));
    icon->setContextMenu(iconMenu);
    icon->show();
    displayMessage("", "FileDistributor is running");
    timer = new QTimer(this);

    connect(icon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    connect(ui->saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui->sourceDirBrowse, SIGNAL(clicked()), this, SLOT(browse()));
    connect(ui->addDirB, SIGNAL(clicked()), this, SLOT(addDest()));
    connect(ui->removeDirB, SIGNAL(clicked()), this, SLOT(removeDest()));
    connect(timer, SIGNAL(timeout()), copier, SLOT(startCopy()));
    connect(this, SIGNAL(startCopy()), copier, SLOT(startCopy()));
    connect(copier, SIGNAL(copyStarted()), this, SLOT(copyStarted()));
    workerThread.start();
    emit startCopy();
    timer->start(1000*copier->getRefreshRateSeconds());
}

DistributorDialog::~DistributorDialog()
{
    workerThread.quit();
    delete ui;
    delete copier;
}

void DistributorDialog::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason)
    {
    case QSystemTrayIcon::Context:
        //context menu handled by QSystemTrayIcon
        break;
    case QSystemTrayIcon::MiddleClick:
        displayMessage();
        break;
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::Unknown:
    default:
        show();
    }
}

void DistributorDialog::displayMessage()
{
     icon->showMessage(lastSubject, lastMessage);
}

void DistributorDialog::displayMessage(const QString &title, const QString &message)
{
    lastMessage = message;
    lastSubject = title;
    displayMessage();
}

void DistributorDialog::quit()
{
    QApplication::quit();
}

void DistributorDialog::browse()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), copier->getSourceDir());

    copier->setSourceDir(dir);
    ui->sourceDirEdit->setText(dir);
}

void DistributorDialog::addDest()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), copier->getSourceDir());

    if (dir.compare(copier->getSourceDir(), Qt::CaseInsensitive) == 0)
    {
        QMessageBox msg;
        msg.setText("Please select a directory other than source");
        msg.exec();
        return;
    }
    copier->addDestDir(dir);

    delete dirsModel;
    dirsModel = new QStringListModel(copier->getDestDirs(), this);
    ui->destList->setModel(dirsModel);
}

void DistributorDialog::removeDest()
{
    QModelIndexList indexes = ui->destList->selectionModel()->selectedIndexes();
    if (indexes.size()>0)
        copier->removeDestDir(indexes.at(0).row());
    delete dirsModel;
    dirsModel = new QStringListModel(copier->getDestDirs(), this);
    ui->destList->setModel(dirsModel);
}

void DistributorDialog::save()
{
    copier->setRefreshRateSeconds(ui->refreshIntervalS->value());
    copier->setSourceDir(ui->sourceDirEdit->text());
    if (timer->interval() != copier->getRefreshRateSeconds()*1000)
    {
        int timeElapsed = timer->interval() - timer->remainingTime();
        if (timeElapsed > copier->getRefreshRateSeconds()*1000)
        {
            timer->stop();
            timer->start(copier->getRefreshRateSeconds()*1000);
            emit startCopy();
        }
        else
        {
            timer->setInterval(copier->getRefreshRateSeconds()*1000);
        }
    }

    QFile settings(dataFileName);
    QFileInfo info(settings);
    if (!info.absoluteDir().exists())
        info.absoluteDir().mkpath(".");
    if (!settings.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox msgBox;
        msgBox.setText("Error Saving Settings to disk");
        msgBox.exec();
        return;
    }

    QTextStream out(&settings);
    out << REFRESH_KEY << "|" << copier->getRefreshRateSeconds() << endl;
    out << SOURCE_DIR_KEY << "|" << copier->getSourceDir() << endl;

    QStringList destDirs = copier->getDestDirs();

    for (int i=0; i<destDirs.length() ; i++)
    {
        out << DEST_DIR_KEY << "|" << destDirs.at(i) << endl;
    }
    settings.close();

    close();
}

void DistributorDialog::load()
{

    QFile settings(dataFileName);
    if (!settings.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&settings);
    QString line = in.readLine();
    QStringList args;
    copier->clearDestDirs();

    while (!line.isNull()) {
        args = line.trimmed().split("|");
        if (args.length() > 1) {
            if (args[0].trimmed().compare(QString(REFRESH_KEY), Qt::CaseInsensitive) == 0)
            {
                copier->setRefreshRateSeconds(args[1].trimmed().toInt());
                ui->refreshIntervalS->setValue(copier->getRefreshRateSeconds());
            }
            if (args[0].trimmed().compare(QString(SOURCE_DIR_KEY), Qt::CaseInsensitive) == 0)
            {
                copier->setSourceDir(args[1].trimmed());
                ui->sourceDirEdit->setText(copier->getSourceDir());
            }
            if (args[0].trimmed().compare(QString(DEST_DIR_KEY), Qt::CaseInsensitive) == 0)
                copier->addDestDir(args[1].trimmed());
        }
        line = in.readLine();
    }
    delete dirsModel;
    dirsModel = new QStringListModel(copier->getDestDirs(), this);
    ui->destList->setModel(dirsModel);
    settings.close();
}

void DistributorDialog::copyStarted()
{
    displayMessage("", "Copying files");
}
