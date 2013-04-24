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

#ifndef REBOOT_LISTENER_H
#define REBOOT_LISTENER_H

#include <QObject>

class KDirWatch;
class QTimer;

class AptRebootListener : public QObject {
  Q_OBJECT
  public:
   AptRebootListener(QObject* parent=0);
  Q_SIGNALS:
   void requestReboot();
  public Q_SLOTS:
   void checkForReboot();
  private Q_SLOTS:
   void slotDirectoryChanged(const QString& path);
  private:
   KDirWatch* m_watcher;
   QTimer* m_timer;
};

#endif // REBOOT_LISTENER_H
