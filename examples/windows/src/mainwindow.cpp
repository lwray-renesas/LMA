#include "mainwindow.hpp"
#include "simulation.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtConcurrent>
#include <iostream>
#include <string>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  p_simulation_params = std::make_unique<SimulationParams>();
  p_simulation_params->stop_simulation = true;

  QChart *p_chart = new QChart;
  p_chart->setTitle("Results");

  p_chart_view = std::make_unique<QChartView>(p_chart, ui->graph_widget);
  p_chart_view->setFixedSize(ui->graph_widget->size());

  p_chart_view->show();
}

MainWindow::~MainWindow()
{
  delete ui;
}

QLineSeries *MainWindow::CreateLineSeriesFromVector(std::vector<int32_t> *p_vec)
{
  QLineSeries *series = new QLineSeries;

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

std::unique_ptr<std::vector<QLineSeries *>> MainWindow::CreateLineSeriesFromVector(std::vector<LMA_Measurements> &p_vec)
{
  auto ret_data = std::make_unique<std::vector<QLineSeries *>>();

  ret_data->emplace_back(new QLineSeries()); // vrms
  ret_data->emplace_back(new QLineSeries()); // irms
  ret_data->emplace_back(new QLineSeries()); // fline
  ret_data->emplace_back(new QLineSeries()); // p
  ret_data->emplace_back(new QLineSeries()); // q
  ret_data->emplace_back(new QLineSeries()); // s

  for (size_t i = 0; i < p_vec.size(); ++i)
  {
    const auto &m = p_vec[i];

    (*ret_data)[0]->append(static_cast<qreal>(i), static_cast<qreal>(m.vrms));
    (*ret_data)[1]->append(static_cast<qreal>(i), static_cast<qreal>(m.irms));
    (*ret_data)[2]->append(static_cast<qreal>(i), static_cast<qreal>(m.fline));
    (*ret_data)[3]->append(static_cast<qreal>(i), static_cast<qreal>(m.p));
    (*ret_data)[4]->append(static_cast<qreal>(i), static_cast<qreal>(m.q));
    (*ret_data)[5]->append(static_cast<qreal>(i), static_cast<qreal>(m.s));
  }

  return std::move(ret_data);
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
                auto chart_combo_text = ui->chart_combobox->currentText().toStdString();

                if ("Input (ADC)" == chart_combo_text)
                {
                  auto voltage_series = CreateLineSeriesFromVector(l_results->raw_voltage_signal.get());
                  auto current_series = CreateLineSeriesFromVector(l_results->raw_current_signal.get());

                  voltage_series->setName(QString("V [ADC]"));
                  current_series->setName(QString("I [ADC]"));
                  p_chart_view->chart()->setTitle(QString("V & I Signals (ADC)"));

                  p_chart_view->chart()->removeAllSeries();
                  p_chart_view->chart()->addSeries(voltage_series);
                  p_chart_view->chart()->addSeries(current_series);
                  p_chart_view->setRubberBand(QChartView::RectangleRubberBand);
                  p_chart_view->chart()->createDefaultAxes();
                }
                else if ("Input (V & I)" == chart_combo_text)
                {
                  auto voltage_series = CreateLineSeriesFromVector(l_results->voltage_signal.get());
                  auto current_series = CreateLineSeriesFromVector(l_results->current_signal.get());

                  voltage_series->setName(QString("V [V]"));
                  current_series->setName(QString("I [A]"));
                  p_chart_view->chart()->setTitle(QString("V & I Signals"));

                  p_chart_view->chart()->removeAllSeries();
                  p_chart_view->chart()->addSeries(voltage_series);
                  p_chart_view->chart()->addSeries(current_series);
                  p_chart_view->setRubberBand(QChartView::RectangleRubberBand);
                  p_chart_view->chart()->createDefaultAxes();
                }
                else
                {
                  auto measurement_series = CreateLineSeriesFromVector(l_results->measurements);
                  if ("Measurements" == chart_combo_text)
                  {
                    (*measurement_series)[0]->setName(QString("Vrms [V]"));
                    (*measurement_series)[1]->setName(QString("Irms [A]"));
                    (*measurement_series)[2]->setName(QString("F line [Hz]"));

                    p_chart_view->chart()->setTitle(QString("Measurement Results"));

                    p_chart_view->chart()->removeAllSeries();
                    p_chart_view->chart()->addSeries((*measurement_series)[0]);
                    p_chart_view->chart()->addSeries((*measurement_series)[1]);
                    p_chart_view->chart()->addSeries((*measurement_series)[2]);
                    p_chart_view->setRubberBand(QChartView::RectangleRubberBand);
                    p_chart_view->chart()->createDefaultAxes();
                  }
                  else if ("Power" == chart_combo_text)
                  {
                    (*measurement_series)[3]->setName(QString("P [W]"));
                    (*measurement_series)[4]->setName(QString("Q [VAR]"));
                    (*measurement_series)[5]->setName(QString("S [VA]"));

                    p_chart_view->chart()->setTitle(QString("Power Results"));

                    p_chart_view->chart()->removeAllSeries();
                    p_chart_view->chart()->addSeries((*measurement_series)[3]);
                    p_chart_view->chart()->addSeries((*measurement_series)[4]);
                    p_chart_view->chart()->addSeries((*measurement_series)[5]);
                    p_chart_view->setRubberBand(QChartView::RectangleRubberBand);
                    p_chart_view->chart()->createDefaultAxes();
                  }
                  else
                  {
                    // Shouldn't get here
                  }
                }

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
