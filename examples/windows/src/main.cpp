#include <QApplication>
#include <mainwindow.hpp>

int main(int argc, const char *argv[])
{
  QApplication app(argc, (char **)argv);

  MainWindow window;
  window.show();

  return app.exec();
}
