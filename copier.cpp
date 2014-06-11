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

#include "copier.h"
#include <QDir>
#include <QFileInfo>

Copier::Copier(QObject *parent) :
    QObject(parent)
{
    sourceDir = "";
    refreshRateSeconds = 60;
}

QString Copier::getSourceDir()
{
    accessor.lock();
    QString ret = sourceDir;
    accessor.unlock();
    return ret;
}

int Copier::getRefreshRateSeconds()
{
    accessor.lock();
    int ret = refreshRateSeconds;
    accessor.unlock();
    return ret;
}

QStringList Copier::getDestDirs()
{
    accessor.lock();
    QStringList ret = destDirs;
    accessor.unlock();
    return ret;
}

void Copier::startCopy()
{
    copier.lock();
    QDir dir(getSourceDir());
    if (getSourceDir().trimmed().length() == 0 || !dir.exists())
    {
        emit message(QString("Error: source directory doesn't exist."));
        copier.unlock();
        return;
    }
    if (destDirs.length() == 0)
    {
        emit message(QString("Error: no destination directories."));
        copier.unlock();
        return;
    }
    //emit copyStarted();

    QStringList fileList = dir.entryList(QDir::Files);

    QString name;
    QString destFolder;
    QFile source, dest;
    int i, extensionIndex;
    QString fileName, extension;
    foreach(name, fileList)
    {
        source.setFileName(dir.absolutePath() + QDir::separator() + name);
        foreach(destFolder, destDirs)
        {
            dest.setFileName(destFolder + QDir::separator() + name);
            if (!source.exists())
                continue;
            i = 1;
            while (dest.exists())
            {
                extensionIndex = name.lastIndexOf(".");
                fileName = name.left(extensionIndex);
                extension = name.right(name.length()-extensionIndex);

                dest.setFileName(destFolder + QDir::separator() + fileName + " (" + QString::number(i) + ")" + extension);
                i++;
            }
            source.copy(dest.fileName());
        }
        dir.remove(name);
    }



    copier.unlock();
}

void Copier::setSourceDir(const QString& dir)
{
    accessor.lock();
    this->sourceDir = dir;
    accessor.unlock();
}

void Copier::setRefreshRateSeconds(int seconds)
{
    accessor.lock();
    refreshRateSeconds = seconds;
    if (refreshRateSeconds < 1)
        refreshRateSeconds = 1;
    accessor.unlock();
}

void Copier::addDestDir(const QString& dir)
{
    accessor.lock();
    destDirs.push_back(dir);
    accessor.unlock();
}

void Copier::removeDestDir(int index)
{
    accessor.lock();
    destDirs.removeAt(index);
    accessor.unlock();
}

void Copier::clearDestDirs()
{
    accessor.lock();
    destDirs.clear();
    accessor.unlock();
}
