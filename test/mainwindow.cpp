#include "mainwindow.h"
#include "ui_mainwindow.h"

#define BAT_MAX_CURRENT_A 400
#define BAT_REG_VOLTAGE 125

static double voltage;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    M_rtCom = new RTCom();
    M_rtCom->Connect("COM255");
    startTimer(20);
    M_values[RTCom::E_RTCom_Sensor_Temp_out] = 214;
    M_values[RTCom::E_RTCom_Sensor_Temp_in] = 281;
    M_values[RTCom::E_RTCom_Sensor_Current] = 2;
    M_values[RTCom::E_RTCom_Sensor_Level_Abwasser] = 10;
    M_values[RTCom::E_RTCom_Sensor_Level_Frischwasser] = 70;
    M_values[RTCom::E_RTCom_Sensor_Voltage_Bat] = 122;
    M_values[RTCom::E_RTCom_Actor_Heizung] = 0;
    M_values[RTCom::E_RTCom_Actor_Klima] = 0;
    M_values[RTCom::E_RTCom_Actor_Magnetventil] = 0;
    M_values[RTCom::E_RTCom_Actor_Music] = 0;
    M_values[RTCom::E_RTCom_Actor_230_Boiler] = 0;
    M_values[RTCom::E_RTCom_Actor_230_Cooker] = 0;
    M_values[RTCom::E_RTCom_Actor_230_Fridge] = 0;
    M_values[RTCom::E_RTCom_Actor_light_bad] = 0;
    M_values[RTCom::E_RTCom_Actor_light_boden] = 0;
    M_values[RTCom::E_RTCom_Actor_light_essen] = 0;
    M_values[RTCom::E_RTCom_Actor_light_kueche] = 0;
    M_values[RTCom::E_RTCom_Actor_light_schlafen] = 0;
    M_values[RTCom::E_RTCom_Actor_light_vorzelt] = 0;
    M_values[RTCom::E_RTCom_Actor_Lima] = 0;
    M_values[RTCom::E_RTCom_Sensor_Netz] = 0;
    M_values[RTCom::E_RTCom_Sensor_Voltage_KFZ] = 124;
    M_values[RTCom::E_RTCom_Actor_230_Fan] = 0;
    voltage = M_values[RTCom::E_RTCom_Sensor_Voltage_Bat] / 10.0;

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    event = event;

    UpdateValues();
}

void MainWindow::UpdateValues()
{
    this->counter ++;
    if ((int) RTCom::E_RTCom_Sensor_Actor_Count == this->counter)
    {
        this->counter = 0;
    }
    if ((int)RTCom::E_RTCom_Sensor_Count == this->counter)
    {
        Simulate();
        return;
    }

    M_rtCom->SetData((RTCom::RTCom_actor_sensor_e)this->counter, M_values[this->counter]);
}

void MainWindow::Simulate()
{
    int sumaryCurrent;
    int resistance_uOhm = BAT_REG_VOLTAGE * 100000 / BAT_MAX_CURRENT_A;

    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Heizung))
    {
        M_values[RTCom::E_RTCom_Actor_Heizung] = 1;
        M_values[RTCom::E_RTCom_Sensor_Temp_in]++;
    }
    else
    {
        M_values[RTCom::E_RTCom_Actor_Heizung] = 0;
    }

    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Klima))
    {
        M_values[RTCom::E_RTCom_Actor_Klima] = 1;
        M_values[RTCom::E_RTCom_Sensor_Temp_in]--;
    }
    else
    {
        M_values[RTCom::E_RTCom_Actor_Klima] = 0;
    }
    sumaryCurrent =  M_values[RTCom::E_RTCom_Sensor_Current];

    voltage += (0.001 * sumaryCurrent);
    M_values[RTCom::E_RTCom_Sensor_Voltage_Bat] = (voltage  + (resistance_uOhm * sumaryCurrent / 1000000)) * 10 ;

     if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Magnetventil))
     {
        M_values[RTCom::E_RTCom_Actor_Magnetventil] = 1;
        M_values[RTCom::E_RTCom_Sensor_Level_Abwasser]--;
     }
     else
     {
         M_values[RTCom::E_RTCom_Actor_Magnetventil] = 0;
     }
     if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Pumpe))
     {
        M_values[RTCom::E_RTCom_Actor_Pumpe] = 1;
     }
     else
     {
         M_values[RTCom::E_RTCom_Actor_Pumpe] = 0;
     }

     this->M_values[RTCom::E_RTCom_Actor_Music] = M_rtCom->GetData(RTCom::E_RTCom_Actor_Music);
     this->M_values[RTCom::E_RTCom_Actor_230_Boiler] = M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Boiler);
     this->M_values[RTCom::E_RTCom_Actor_230_Cooker] = M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Cooker);
     this->M_values[RTCom::E_RTCom_Actor_230_Fridge] = M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fridge);
     this->M_values[RTCom::E_RTCom_Actor_230_Fan] = M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fan);
     this->M_values[RTCom::E_RTCom_Actor_light_bad] = M_rtCom->GetData(RTCom::E_RTCom_Actor_light_bad);
     this->M_values[RTCom::E_RTCom_Actor_light_boden] = M_rtCom->GetData(RTCom::E_RTCom_Actor_light_boden);
     this->M_values[RTCom::E_RTCom_Actor_light_essen] = M_rtCom->GetData(RTCom::E_RTCom_Actor_light_essen);
     this->M_values[RTCom::E_RTCom_Actor_light_kueche] = M_rtCom->GetData(RTCom::E_RTCom_Actor_light_kueche);
     this->M_values[RTCom::E_RTCom_Actor_light_schlafen] = M_rtCom->GetData(RTCom::E_RTCom_Actor_light_schlafen);
     this->M_values[RTCom::E_RTCom_Actor_light_vorzelt] = M_rtCom->GetData(RTCom::E_RTCom_Actor_light_vorzelt);
     this->M_values[RTCom::E_RTCom_Actor_Lima] = M_rtCom->GetData(RTCom::E_RTCom_Actor_Lima);

}



void MainWindow::on_pushButton_4_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Current] = valueInt;
}

void MainWindow::on_pushButton_5_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Voltage_Bat] = valueInt;
    voltage = M_values[RTCom::E_RTCom_Sensor_Voltage_Bat] / 10.0;
}

void MainWindow::on_pushButton_6_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Level_Frischwasser] = valueInt;
}

void MainWindow::on_pushButton_7_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Level_Abwasser] = valueInt;
}

void MainWindow::on_pushButton_8_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Temp_in] = valueInt;
}

void MainWindow::on_pushButton_9_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Temp_out] = valueInt;
}

void MainWindow::on_pushButton_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Voltage_KFZ] = valueInt;
}

void MainWindow::on_pushButton_2_clicked()
{
    QString value = ui->lineEdit_analogValue->text();
    int valueInt = value.toInt();
    M_values[RTCom::E_RTCom_Sensor_Netz] = (0 != valueInt) ? 1 : 0;
}
