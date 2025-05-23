#include "mainwindow.hpp"
#include "simulation.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtConcurrent>
#include <string>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  simulation_params = std::make_unique<SimulationParams>();
  simulation_params->stop_simulation = true;
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
    simulation_params->stop_simulation = false;
    simulation_params->sample_count = std::stoull(ui->sample_count_input->text().toStdString());
    simulation_params->ps = std::stod(ui->phase_shift_input->text().toStdString());
    simulation_params->vrms = std::stod(ui->vrms_input->text().toStdString());
    simulation_params->irms = std::stod(ui->irms_input->text().toStdString());
    simulation_params->fs = std::stod(ui->sample_frequency_input->text().toStdString());
    simulation_params->fline = std::stod(ui->line_frequency_input->text().toStdString());
    simulation_params->calibrate = ui->calibrate_checkbox->isChecked();
    simulation_params->rogowski = ui->rogowski_checkbox->isChecked();

    // Launch the simulation in another thread
    auto simulation_future = QtConcurrent::run(
        [=]()
        {
          Simulation(simulation_params.get());

          QMetaObject::invokeMethod(
              this,
              [=]()
              {
                sim_running = false;
                simulation_params->stop_simulation = false;
                ui->simulate_button->setText("Simulate");
                ui->simulate_button->setEnabled(true);
              },
              Qt::QueuedConnection);
        });
  }
  else
  {
    simulation_params->stop_simulation = true;
    ui->simulate_button->setEnabled(false);
  }
}
