#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <memory>
#include "simulation.hpp"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

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
  //
  // @brief Create a QLineSeries from a vector of int32_t
  // @param[in] - p_vec - pointer to the vector to populate.
  //
  QLineSeries *CreateLineSeriesFromVector(std::vector<int32_t> *p_vec);
  QLineSeries *CreateLineSeriesFromVector(std::vector<double> *p_vec);

  Ui::MainWindow *ui;
  std::unique_ptr<SimulationParams> p_simulation_params;
  std::unique_ptr<QChartView> p_chart_view;

private slots:
  void on_simulate_button_clicked();
};

#endif // MAINWINDOW_HPP
