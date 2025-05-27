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
  p_chart->setTitle("Simple Line Chart");

  p_chart_view = std::make_unique<QChartView>(p_chart, ui->graph_widget);
  p_chart_view->setFixedSize(ui->graph_widget->size());

  p_chart_view->show();
}

MainWindow::~MainWindow()
{
  delete ui;
}

QLineSeries* MainWindow::CreateLineSeriesFromVector(std::vector<int32_t>* p_vec)
{
  QLineSeries * series = new QLineSeries;

  for (size_t i = 0; i < p_vec->size(); ++i)
  {
    series->append(static_cast<qreal>(i), static_cast<qreal>((*p_vec)[i]));
  }

  return series;
}

QLineSeries *MainWindow::CreateLineSeriesFromVector(std::vector<double> *p_vec)
{
  QLineSeries *series = new QLineSeries;

  for (size_t i = 0; i < p_vec->size(); ++i)
  {
    series->append(static_cast<qreal>(i), static_cast<qreal>((*p_vec)[i]));
  }

  return series;
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
          auto l_results = Simulation(p_simulation_params.get());

          QMetaObject::invokeMethod(
              this,
              [=]()
              {
                auto voltage_series = CreateLineSeriesFromVector(l_results->raw_voltage_signal.get());
                auto current_series = CreateLineSeriesFromVector(l_results->raw_current_signal.get());

                // Plot on chart
                p_chart_view->chart()->removeAllSeries();
                p_chart_view->chart()->addSeries(dynamic_cast<QAbstractSeries *>(voltage_series));
                p_chart_view->chart()->addSeries(dynamic_cast<QAbstractSeries *>(current_series));
                p_chart_view->setRubberBand(QChartView::RectangleRubberBand);
                p_chart_view->chart()->createDefaultAxes();

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
