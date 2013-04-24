/* Copyright (c) 2010 Sune Vuorela <sune@vuorela.dk>
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE. */

#include "RebootListener.h"
#include <QFileSystemWatcher>
#include <QDebug>
#include <KDirWatch>
#include <QTimer>
#include <QFile>

static const char reboot_required_path[] = "/run/reboot-required";

AptRebootListener::AptRebootListener(QObject* parent): QObject(parent) {
  m_watcher = new KDirWatch(this);
  m_watcher->addFile(QString::fromLatin1(reboot_required_path));
  connect(m_watcher,SIGNAL(created(QString)),this,SLOT(slotDirectoryChanged(QString)));
  m_timer = new QTimer(this);
  m_timer->setSingleShot(true);
  m_timer->setInterval(500);
  connect(m_timer,SIGNAL(timeout()),SIGNAL(requestReboot()));
}

void AptRebootListener::checkForReboot() {
  if(QFile::exists(QString::fromLatin1(reboot_required_path))) {
    m_timer->start();
  }
}


void AptRebootListener::slotDirectoryChanged(const QString& path) {
  if(path==QLatin1String(reboot_required_path)) {
     m_timer->start();
  }
}

#include <RebootListener.moc>
