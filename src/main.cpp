#include "snowoverlay.h"
#include "treewidget.h"
#include <QApplication>
#include <QScreen>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  TreeWidget *tree = new TreeWidget();
  SnowOverlay *backSnow = new SnowOverlay(false);
  SnowOverlay *frontSnow = new SnowOverlay(true);

  tree->setSnowLayers(backSnow, frontSnow);

  QScreen *screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->availableGeometry();

  // Exact center
  int x = screenGeometry.left() + (screenGeometry.width() - 400) / 2;
  int y = screenGeometry.top() + (screenGeometry.height() - 500) / 2;

  tree->move(x, y);
  backSnow->move(x, 0);  // Start at top, align horizontally
  frontSnow->move(x, 0); // Start at top, align horizontally

  backSnow->show();
  tree->show();
  frontSnow->show();

  // Enforce front snow on top of the tree with a small delay to handle macOS
  // window manager
  QTimer::singleShot(100, [frontSnow]() { frontSnow->raise(); });

  return a.exec();
}
