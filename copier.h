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

#ifndef COPIER_H
#define COPIER_H

#include <QObject>
#include <QStringList>
#include <QMutex>

class Copier : public QObject
{
    Q_OBJECT
public:
    explicit Copier(QObject *parent = 0);

    QString getSourceDir();
    int getRefreshRateSeconds();
    QStringList getDestDirs();



signals:
    void copyStarted();
    void message(const QString&);

public slots:
    void startCopy();
    void setSourceDir(const QString &dir);
    void setRefreshRateSeconds(int seconds);
    void addDestDir(const QString &dir);
    void removeDestDir(int index);
    void clearDestDirs();

private:
    QString sourceDir;
    QStringList destDirs;
    int refreshRateSeconds;

    QMutex accessor;
    QMutex copier;

};

#endif // COPIER_H
