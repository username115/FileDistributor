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

#ifndef DISTRIBUTORDIALOG_H
#define DISTRIBUTORDIALOG_H

#include <QSystemTrayIcon>
#include <QDialog>
#include <QMenu>
#include <QStringListModel>
#include <QThread>
#include <QTimer>
#include "copier.h"

namespace Ui {
class DistributorDialog;
}

class DistributorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DistributorDialog(QWidget *parent = 0);
    ~DistributorDialog();

    //void setVisible(bool visible);

signals:
    void startCopy();
public slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);
    void displayMessage();
    void displayMessage(const QString& title, const QString& message);
    void quit();

    void browse();
    void addDest();
    void removeDest();
    void save();

    void load();

    void copyStarted();

protected:
    // void closeEvent(QCloseEvent *event);

private:

    QSystemTrayIcon* icon;
    QMenu*  iconMenu;
    QString lastSubject;
    QString lastMessage;
    QString dataFileName;


    QStringListModel* dirsModel;


    QTimer* timer;
    Copier* copier;
    QThread workerThread;

    Ui::DistributorDialog *ui;
};

#endif // DISTRIBUTORDIALOG_H
