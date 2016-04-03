#include "mainwindow.h"
#include "ui_mainwindow.h"

#define BAT_MAX_CURRENT_A 400
#define BAT_REG_VOLTAGE 125
#define ABWASSER_MAX    70
#define WASSER_MAX  70
#define LIMA_MIN_VOLTAGE 144
#define BAT_MAX_CHARGE_VOLTAGE 144

static const int S_capTable[10][3] = {
    {10,118,10},
    {20,119,10},
    {30,121,20},
    {40,122,20},
    {50,123,60},
    {60,125,60},
    {70,126,60},
    {80,127,80},
    {90,128,80},
    {100,130,100}
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    M_rtCom = new RTCom();
    M_rtCom->Connect("COM254");
    timerBusy = false;
    M_cookerSwitchOn = false;
    M_lastStateBoiler = 0;
    M_lastStateFridge = 0;

    ui->setupUi(this);
    ui->label_heating_On->setVisible(false);
    ui->label_cooling_On->setVisible(false);
    ui->label_wasserLassen->setVisible(false);
    ui->label_connected->setVisible(false);
    ui->ActorBoilerLabelOn->setVisible(false);
    ui->ActorCookerLabelOn->setVisible(false);
    ui->ActorFridgeLabelOn->setVisible(false);
    ui->ActorMusicLabelOn->setVisible(false);
    ui->ActorFanLabelOn->setVisible(false);

    ui->switchOffBad->setVisible(false);
    ui->switchOffBoden->setVisible(false);
    ui->switchOffEssen->setVisible(false);
    ui->switchOffKueche->setVisible(false);
    ui->switchOffSchlafen->setVisible(false);
    ui->switchOffVorzelt->setVisible(false);

    ui->switchOnBad->setVisible(false);
    ui->switchOnBoden->setVisible(false);
    ui->switchOnEssen->setVisible(false);
    ui->switchOnKueche->setVisible(false);
    ui->switchOnSchlafen->setVisible(false);
    ui->switchOnVorzelt->setVisible(false);

    this->M_batteryValues.input = 0;
    this->M_batteryValues.output = 0;
    this->M_batteryValues.summe = 0;
    this->M_batteryValues.numBateries = 4;
    this->M_batteryValues.voltage = 129;

    this->M_homeValues.tempIn = 16.1;
    this->M_homeValues.tempOut = 10.5;
    this->M_homeValues.tempInSoll = 20.0;
    this->M_homeValues.heatingOn = false;
    this->M_homeValues.airconOn = false;

    this->M_waterPage.abwasserL = 23;
    this->M_waterPage.frischwasserL = 30;
    this->M_waterPage.abwasserSollL = 0;
    this->M_waterPage.Pumpe = false;
    this->M_waterPage.magnetVentil = false;

    startTimer(500);

    UpdateBatteryValues();
    UpdateHomeValues();
    UpdateWaterValues();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::UpdateBatteryValues (void)
{
    int chargingLevel;

    this->M_batteryValues.summe = M_rtCom->GetData(RTCom::E_RTCom_Sensor_Current);
    this->M_batteryValues.voltage = M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_Bat);
    this->M_batteryValues.voltageKfz = M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_KFZ);
    this->M_batteryValues.netz = M_rtCom->GetData(RTCom::E_RTCom_Sensor_Netz);
    this->M_batteryValues.lima = M_rtCom->GetData(RTCom::E_RTCom_Actor_Lima);

    ui->lcd_Voltage->display(this->M_batteryValues.voltage / 10);
    ui->lcd_VoltageTenth->display(this->M_batteryValues.voltage % 10);

    ui->lcd_Voltage_KFZ->display(this->M_batteryValues.voltageKfz / 10);
    ui->lcd_VoltageKFZTenth->display(this->M_batteryValues.voltageKfz % 10);

    if (0 > this->M_batteryValues.summe)
    {
        this->M_batteryValues.output = -this->M_batteryValues.summe;
        this->M_batteryValues.input = 0;
        ui->lcd_chargeAbsolute->display(this->M_batteryValues.output);
        ui->ArrowGreen->setVisible(false);
        ui->ArrowRed->setVisible(true);
    }
    else if (0 == this->M_batteryValues.summe)
    {
        this->M_batteryValues.output = 0;
        this->M_batteryValues.input = 0;
        ui->lcd_chargeAbsolute->display(0);
        ui->ArrowGreen->setVisible(false);
        ui->ArrowRed->setVisible(false);
    }
    else
    {
        this->M_batteryValues.output = 0;
        this->M_batteryValues.input = this->M_batteryValues.summe;
        ui->lcd_chargeAbsolute->display(this->M_batteryValues.input);
        ui->ArrowGreen->setVisible(true);
        ui->ArrowRed->setVisible(false);
    }

    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Sensor_Netz))
    {
        ui->Sensor_Netz_Off->setVisible(false);
        ui->Sensor_Netz_On->setVisible(true);
    }
    else
    {
        ui->Sensor_Netz_On->setVisible(false);
        ui->Sensor_Netz_Off->setVisible(true);
    }

    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Lima))
    {
        ui->Sensor_Lima_Off->setVisible(false);
        ui->Sensor_Lima_On->setVisible(true);
        ui->progressBarLima->setValue(M_rtCom->GetData(RTCom::E_RTCom_Actor_Lima));
    }
    else
    {
        ui->Sensor_Lima_On->setVisible(false);
        ui->Sensor_Lima_Off->setVisible(true);
        ui->progressBarLima->setValue(0);
    }

    chargingLevel = CalculateLevel();
    ui->lcdCapacity->display(S_capTable[chargingLevel][0]);
    ui->labelBat0->setVisible(false);
    ui->labelBat20->setVisible(false);
    ui->labelBat60->setVisible(false);
    ui->labelBat80->setVisible(false);
    ui->labelBat100->setVisible(false);
    /* update bat symbol    */
    ui->label_homeBat0->setVisible(false);
    ui->label_homeBat20->setVisible(false);
    ui->label_homeBat60->setVisible(false);
    ui->label_homeBat80->setVisible(false);
    ui->label_homeBat100->setVisible(false);

    switch (S_capTable[chargingLevel][2])
    {
    case 10:
        ui->labelBat0->setVisible(true);
        ui->label_homeBat0->setVisible(true);
        break;
    case 20:
        ui->labelBat20->setVisible(true);
        ui->label_homeBat20->setVisible(true);
        break;
    case 60:
        ui->labelBat60->setVisible(true);
        ui->label_homeBat60->setVisible(true);
        break;
    case 80:
        ui->labelBat80->setVisible(true);
        ui->label_homeBat80->setVisible(true);
        break;
    case 100:
        ui->labelBat100->setVisible(true);
        ui->label_homeBat100->setVisible(true);
        break;
    default:
        ui->labelBat0->setVisible(true);
        ui->label_homeBat0->setVisible(true);
        break;
    }

    if (LIMA_MIN_VOLTAGE < M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_KFZ))
    {
        int limaValue = M_rtCom->GetData(RTCom::E_RTCom_Actor_Lima);

        limaValue++;

        if (255 >= limaValue)
        {
            M_rtCom->SetData(RTCom::E_RTCom_Actor_Lima, limaValue);
        }
    }
    else if (   (M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_KFZ) < M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_Bat))
             || (BAT_MAX_CHARGE_VOLTAGE <= M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_Bat)))
    {
       M_rtCom->SetData(RTCom::E_RTCom_Actor_Lima, 0u);
    }
    else if (LIMA_MIN_VOLTAGE > M_rtCom->GetData(RTCom::E_RTCom_Sensor_Voltage_KFZ))
    {
        int limaValue = M_rtCom->GetData(RTCom::E_RTCom_Actor_Lima);

        limaValue--;

        if (0 <= limaValue)
        {
            M_rtCom->SetData(RTCom::E_RTCom_Actor_Lima, limaValue);
        }
    }
}

void MainWindow::UpdateHomeValues (void)
{
    this->M_homeValues.tempIn = (M_rtCom->GetData(RTCom::E_RTCom_Sensor_Temp_in) / 10.0);
    this->M_homeValues.tempOut = (M_rtCom->GetData(RTCom::E_RTCom_Sensor_Temp_out) / 10.0);

    ui->lcdDegreeIn->display(this->M_homeValues.tempIn);
    ui->lcdDegreeOut->display(this->M_homeValues.tempOut);
    this->M_homeValues.tempInSoll = ui->dialTemp->value()/10.0;

    if(     (this->M_homeValues.tempIn < this->M_homeValues.tempInSoll)
       &&   (this->M_homeValues.heatingOn))
    {
      //  this->SetKlima(false);
      //  this->SetHeizung(true);
    }
    else if (   (this->M_homeValues.tempIn > this->M_homeValues.tempInSoll)
              &&(this->M_homeValues.airconOn))
    {
      //  this->SetHeizung(false);
      //  this->SetKlima(true);
    }
    else
    {
      //  this->SetHeizung(false);
      //  this->SetKlima(false);
    }
    if(0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Heizung))
    {
        ui->label_heating->setVisible(true);
    }
    else
    {
        ui->label_heating->setVisible(false);
    }
    if(0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Klima))
    {
        ui->label_cooling->setVisible(true);
    }
    else
    {
        ui->label_cooling->setVisible(false);
    }
}

void MainWindow::UpdateWaterValues (void)
{
    this->M_waterPage.abwasserL = M_rtCom->GetData(RTCom::E_RTCom_Sensor_Level_Abwasser);
    this->M_waterPage.frischwasserL = M_rtCom->GetData(RTCom::E_RTCom_Sensor_Level_Frischwasser);
    ui->progressBarAbwasser->setValue(this->M_waterPage.abwasserL);
    ui->progressBarFrischwasser->setValue(this->M_waterPage.frischwasserL);
    ui->lcdNumberAbwasser->display(this->M_waterPage.abwasserL);
    ui->lcdNumberFrischwasser->display(this->M_waterPage.frischwasserL);
    ui->ScrollBarWasserAblassen->setMaximum(this->M_waterPage.abwasserL);

    if(     (this->M_waterPage.magnetVentil)
         && (this->M_waterPage.abwasserL <= this->M_waterPage.abwasserSollL))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Magnetventil, 1);
    }
    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Magnetventil))
    {
        ui->label_wasserLassen->setVisible(false);
        this->M_waterPage.magnetVentil = false;
    }

    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Pumpe))
    {
        ui->label_PumpeStat->setText("ON");
    }
    else
    {
        ui->label_PumpeStat->setText("OFF");
    }

    if (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Magnetventil))
    {
        ui->label_wasserLassen->setVisible(true);

        if (this->M_waterPage.abwasserSollL >= this->M_waterPage.abwasserL)
        {
            M_rtCom->SetData(RTCom::E_RTCom_Actor_Magnetventil, 0);
        }
    }
    else
    {
        ui->label_wasserLassen->setVisible(false);
    }


}

void MainWindow::UpdateActorValues (void)
{
    if (    (this->M_cookerSwitchOn)
        &&  (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Boiler))
        &&  (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fridge)))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Cooker, 1);
    }
    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_Music))
    {
        ui->ActorMusicLabelOff->setVisible(false);
        ui->ActorMusicLabelOn->setVisible(true);
    }
    else
    {
        ui->ActorMusicLabelOff->setVisible(true);
        ui->ActorMusicLabelOn->setVisible(false);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Cooker))
    {
        ui->ActorCookerLabelOff->setVisible(false);
        ui->ActorCookerLabelOn->setVisible(true);
        this->M_cookerSwitchOn = false;
    }
    else
    {
        ui->ActorCookerLabelOff->setVisible(true);
        ui->ActorCookerLabelOn->setVisible(false);
        if (    (0 != M_lastStateBoiler)
            &!  (this->M_cookerSwitchOn))
        {
            M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Boiler, M_lastStateBoiler);
            ui->ActorBoilerButton->setDisabled(false);
            M_lastStateBoiler = 0;
        }
        if (    (0 != M_lastStateFridge)
            &!  (this->M_cookerSwitchOn))
        {
            M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Fridge, M_lastStateFridge);
            ui->ActorFridgeButton->setDisabled(false);
            M_lastStateFridge = 0;
        }
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Boiler))
    {
        ui->ActorBoilerLabelOff->setVisible(false);
        ui->ActorBoilerLabelOn->setVisible(true);
    }
    else
    {
        ui->ActorBoilerLabelOff->setVisible(true);
        ui->ActorBoilerLabelOn->setVisible(false);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fridge))
    {
        ui->ActorFridgeLabelOff->setVisible(false);
        ui->ActorFridgeLabelOn->setVisible(true);
    }
    else
    {
        ui->ActorFridgeLabelOff->setVisible(true);
        ui->ActorFridgeLabelOn->setVisible(false);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_light_bad))
    {
        ui->switchOffBad->setVisible(false);
        ui->switchOnBad->setVisible(true);
    }
    else
    {
        ui->switchOnBad->setVisible(false);
        ui->switchOffBad->setVisible(true);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_light_boden))
    {
        ui->switchOffBoden->setVisible(false);
        ui->switchOnBoden->setVisible(true);
    }
    else
    {
        ui->switchOnBoden->setVisible(false);
        ui->switchOffBoden->setVisible(true);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_light_essen))
    {
        ui->switchOffEssen->setVisible(false);
        ui->switchOnEssen->setVisible(true);
    }
    else
    {
        ui->switchOnEssen->setVisible(false);
        ui->switchOffEssen->setVisible(true);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_light_kueche))
    {
        ui->switchOffKueche->setVisible(false);
        ui->switchOnKueche->setVisible(true);
    }
    else
    {
        ui->switchOnKueche->setVisible(false);
        ui->switchOffKueche->setVisible(true);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_light_schlafen))
    {
        ui->switchOffSchlafen->setVisible(false);
        ui->switchOnSchlafen->setVisible(true);
    }
    else
    {
        ui->switchOnSchlafen->setVisible(false);
        ui->switchOffSchlafen->setVisible(true);
    }

    if(M_rtCom->GetData(RTCom::E_RTCom_Actor_light_vorzelt))
    {
        ui->switchOffVorzelt->setVisible(false);
        ui->switchOnVorzelt->setVisible(true);
    }
    else
    {
        ui->switchOnVorzelt->setVisible(false);
        ui->switchOffVorzelt->setVisible(true);
    }

}

int MainWindow::CalculateLevel (void)
{
    int resistance_uOhm = BAT_REG_VOLTAGE * 100000 / BAT_MAX_CURRENT_A;
    int voltageBat_zeroAmp = ((-this->M_batteryValues.summe) * resistance_uOhm /100000) + this->M_batteryValues.voltage;

    for(int i=9; i>=0;i--)
    {
        if (S_capTable[i][1] <= voltageBat_zeroAmp)
        {
            return i;
        }

    }
    return 0;
}

void MainWindow::on_dialTemp_sliderMoved(int position)
{
    position = position;
    ui->lcdDegreeInSoll->display((double)ui->dialTemp->value()/10.0);
    UpdateHomeValues();
}

void MainWindow::on_pushTempUp_clicked()
{
    double actValue = (double)ui->dialTemp->value()/10.0;
    actValue += 0.1;
    ui->lcdDegreeInSoll->display(actValue);
    ui->dialTemp->setValue(actValue*10);
    UpdateHomeValues();
}

void MainWindow::on_pushTempDown_clicked()
{
    double actValue = (double)ui->dialTemp->value()/10.0;
    actValue -= 0.1;
    ui->lcdDegreeInSoll->display(actValue);
    ui->dialTemp->setValue(actValue*10);
    UpdateHomeValues();
}

void MainWindow::on_toolButtonHeizung_clicked()
{
    if(this->M_homeValues.heatingOn)
    {
        this->M_homeValues.heatingOn = false;
        ui->label_heating_On->setVisible(false);
    }
    else
    {
        this->M_homeValues.heatingOn = true;
        ui->label_heating_On->setVisible(true);
    }
    UpdateHomeValues();
}

void MainWindow::on_toolButtonKlima_clicked()
{
    if(this->M_homeValues.airconOn)
    {
        this->M_homeValues.airconOn = false;
        ui->label_cooling_On->setVisible(false);
    }
    else
    {
        this->M_homeValues.airconOn = true;
        ui->label_cooling_On->setVisible(true);
    }
    UpdateHomeValues();
}


void MainWindow::on_ScrollBarWasserAblassen_valueChanged(int value)
{
    ui->lcdNumberAbwasserSoll->display(value);
    this->M_waterPage.abwasserSollL = this->M_waterPage.abwasserL - value;
    UpdateWaterValues();
}

void MainWindow::on_toolButton_clicked()
{
    if(0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Pumpe))
    {
        this->M_waterPage.Pumpe = false;
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Pumpe, 0);
    }
    else
    {
        this->M_waterPage.Pumpe = true;
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Pumpe, 1);
    }
}

void MainWindow::on_toolButton_2_clicked()
{
    if(0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Magnetventil))

    {
        this->M_waterPage.magnetVentil = false;
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Magnetventil, 0);
    }
    else if (this->M_waterPage.abwasserSollL < this->M_waterPage.abwasserL)
    {
        this->M_waterPage.magnetVentil = true;
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Magnetventil, 1);
    }
    UpdateWaterValues();
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    if (this->timerBusy)
        return;
    timerBusy = true;
    event = event;
    this->UpdateBatteryValues();
    this->UpdateHomeValues();
    this->UpdateWaterValues();
    this->UpdateActorValues();

    if (!M_rtCom->isConnected())
    {
        if (M_rtCom->Connect("COM254"))
        {
            ui->label_connected->setVisible(true);
        }
        else
        {
            ui->label_connected->setVisible(false);
        }
    }
    else
    {
        ui->label_connected->setVisible(true);
    }
    timerBusy = false;
}

void MainWindow::on_ActorCookerButton_clicked()
{
    if (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Cooker))
    {
        this->M_cookerSwitchOn = true;
        M_lastStateBoiler = M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Boiler);
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Boiler, 0);
        M_lastStateFridge = M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fridge);
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Fridge, 0);
        ui->ActorBoilerButton->setDisabled(true);
        ui->ActorFridgeButton->setDisabled(true);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Cooker, 0);
    }
}

void MainWindow::on_ActorBoilerButton_clicked()
{
    if (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Boiler))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Boiler, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Boiler, 0);
    }
}

void MainWindow::on_ActorMusicButton_clicked()
{
    if (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_Music))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Music, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Music, 0);
    }
}

void MainWindow::on_ActorFridgeButton_clicked()
{
    if (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fridge))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Fridge, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Fridge, 0);
    }
}

void MainWindow::on_ActorFanButton_clicked()
{
    if (0 == M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Fan))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Fan, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_230_Fan, 0);
    }
}

void MainWindow::SetKlima(bool on)
{
    /* switch off if necessary  */
    if (    (!on)
        &&  (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Klima)))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Klima, 0);
        return;
    }

    /* switch on if necessary and allowed   */
    if (    (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Cooker))
        &&  (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Klima))
        &&  (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Heizung)))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Klima, 1);
    }
}

void MainWindow::SetHeizung(bool on)
{
    /* switch off if necessary  */
    if (    (!on)
        &&  (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Heizung)))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Heizung, 0);
        return;
    }

    /* switch on if necessary and allowed   */
    if (    (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_230_Cooker))
        &&  (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Klima))
        &&  (0 != M_rtCom->GetData(RTCom::E_RTCom_Actor_Heizung)))
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_Heizung, 1);
    }
}


void MainWindow::on_pushButtonLichtEssen_clicked()
{
    if(ui->switchOffEssen->isVisible())
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_essen, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_essen, 0);
    }
}

void MainWindow::on_pushButtonLichtKueche_clicked()
{
    if(ui->switchOffKueche->isVisible())
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_kueche, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_kueche, 0);
    }
}

void MainWindow::on_pushButtonLichtBad_clicked()
{
    if(ui->switchOffBad->isVisible())
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_bad, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_bad, 0);
    }
}

void MainWindow::on_pushButtonLichtBoden_clicked()
{
    if(ui->switchOffBoden->isVisible())
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_boden, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_boden, 0);
    }
}

void MainWindow::on_pushButtonLichtSchlafen_clicked()
{
    if(ui->switchOffSchlafen->isVisible())
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_schlafen, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_schlafen, 0);
    }
}

void MainWindow::on_pushButtonLichtVorzelt_clicked()
{
    if(ui->switchOffVorzelt->isVisible())
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_vorzelt, 1);
    }
    else
    {
        M_rtCom->SetData(RTCom::E_RTCom_Actor_light_vorzelt, 0);
    }
}

void MainWindow::on_pushButtonLichtZentral_clicked()
{
    M_rtCom->SetData(RTCom::E_RTCom_Actor_light_essen, 0);
    M_rtCom->SetData(RTCom::E_RTCom_Actor_light_kueche, 0);
    M_rtCom->SetData(RTCom::E_RTCom_Actor_light_bad, 0);
    M_rtCom->SetData(RTCom::E_RTCom_Actor_light_boden, 0);
    M_rtCom->SetData(RTCom::E_RTCom_Actor_light_schlafen, 0);
    M_rtCom->SetData(RTCom::E_RTCom_Actor_light_vorzelt, 0);
}


