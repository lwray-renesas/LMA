#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>
#include "simulation.hpp"

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  Ui::MainWindow *ui;
  std::unique_ptr<SimulationParams> simulation_params;

private slots:
  void on_simulate_button_clicked();
};

#endif // MAINWINDOW_HPP
