#include "mainwindow.hpp"
#include "simulation.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtConcurrent>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  p_simulation_params = std::make_unique<SimulationParams>();
  p_simulation_params->stop_simulation = true;

  QChart *p_chart = new QChart;
  p_chart->createDefaultAxes();
  p_chart->setTitle("Simple Line Chart");

  p_chart_view = std::make_unique<QChartView>(p_chart, ui->graph_widget);
  p_chart_view->setFixedSize(ui->graph_widget->size());

  p_chart_view->show();
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::on_simulate_button_clicked()
{
  // Track the status of the simulations running
  static bool sim_running = false;

  if (!sim_running)
  {
    sim_running = true;
    ui->simulate_button->setText("Cancel");
    p_simulation_params->stop_simulation = false;
    p_simulation_params->sample_count = std::stoull(ui->sample_count_input->text().toStdString());
    p_simulation_params->ps = std::stod(ui->phase_shift_input->text().toStdString());
    p_simulation_params->vrms = std::stod(ui->vrms_input->text().toStdString());
    p_simulation_params->irms = std::stod(ui->irms_input->text().toStdString());
    p_simulation_params->fs = std::stod(ui->sample_frequency_input->text().toStdString());
    p_simulation_params->fline = std::stod(ui->line_frequency_input->text().toStdString());
    p_simulation_params->calibrate = ui->calibrate_checkbox->isChecked();
    p_simulation_params->rogowski = ui->rogowski_checkbox->isChecked();

    // Launch the simulation in another thread
    auto simulation_future = QtConcurrent::run(
        [=]()
        {
          Simulation(p_simulation_params.get());

          // Plot on chart
          auto p_line_series = new QLineSeries;
          p_line_series->append(0, 6);
          p_line_series->append(2, 4);
          p_line_series->append(3, 8);
          p_line_series->append(7, 4);
          p_line_series->append(10, 5);
          *p_line_series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

          p_chart_view->chart()->removeAllSeries();
          p_chart_view->chart()->addSeries(dynamic_cast<QAbstractSeries *>(p_line_series));

          QMetaObject::invokeMethod(
              this,
              [=]()
              {
                sim_running = false;
                p_simulation_params->stop_simulation = false;
                ui->simulate_button->setText("Simulate");
                ui->simulate_button->setEnabled(true);
              },
              Qt::QueuedConnection);
        });
  }
  else
  {
    p_simulation_params->stop_simulation = true;
    ui->simulate_button->setEnabled(false);
  }
}
