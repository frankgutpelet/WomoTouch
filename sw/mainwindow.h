#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "rtcom.h"
#include <QPushButton>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    typedef struct
    {
        int summe;
        int input;
        int output;
        int voltage;
        int voltageKfz;
        int netz;
        int lima;
        int numBateries;
    } BatteryPage_t;

    typedef struct
    {
        double tempIn;
        double tempOut;
        double tempInSoll;
        bool heatingOn;
        bool airconOn;
    } HomePage_t;

    typedef struct
    {
        int frischwasserL;
        int abwasserL;
        int abwasserSollL;
        bool Pumpe;
        bool magnetVentil;
    } WaterPage_t;

    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void UpdateBatteryValues (void);
    void UpdateHomeValues (void);
    void UpdateWaterValues (void);
    void UpdateActorValues (void);

protected:
    void timerEvent(QTimerEvent *event);

private slots:

    void on_dialTemp_sliderMoved(int position);

    void on_pushTempUp_clicked();

    void on_pushTempDown_clicked();

    void on_toolButtonHeizung_clicked();

    void on_toolButtonKlima_clicked();

    void on_ScrollBarWasserAblassen_valueChanged(int value);

    void on_toolButton_clicked();

    void on_toolButton_2_clicked();

    void on_ActorCookerButton_clicked();

    void on_ActorBoilerButton_clicked();

    void on_ActorMusicButton_clicked();

    void on_ActorFridgeButton_clicked();

    void on_pushButtonLichtEssen_clicked();

    void on_pushButtonLichtKueche_clicked();

    void on_pushButtonLichtBad_clicked();

    void on_pushButtonLichtBoden_clicked();

    void on_pushButtonLichtSchlafen_clicked();

    void on_pushButtonLichtVorzelt_clicked();

    void on_pushButtonLichtZentral_clicked();

    void on_ActorFanButton_clicked();

private:
    Ui::MainWindow *ui;
    BatteryPage_t M_batteryValues;
    HomePage_t M_homeValues;
    WaterPage_t M_waterPage;
    RTCom *M_rtCom;
    bool timerBusy;
    bool M_cookerSwitchOn;
    int M_lastStateBoiler;
    int M_lastStateFridge;

    int CalculateLevel (void);
    void SetKlima(bool on);
    void SetHeizung(bool on);
    void UpdateUI (RTCom::RTCom_actor_sensor_e actor, QLabel* on, QLabel* off);

};

#endif // MAINWINDOW_H
