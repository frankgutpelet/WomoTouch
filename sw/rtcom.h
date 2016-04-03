#ifndef RTCOM_H
#define RTCOM_H

#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QMutex>

#define START_OF_FRAME  (char) 0xBB
#define MAX_MESSAGE_LENGTH 14
#define TX_BUFFER_LENGTH 10

class RTCom : public QThread
{
public:

    typedef enum
    {
        E_RTCom_Sensor_Voltage_Bat,
        E_RTCom_Sensor_Current,
        E_RTCom_Sensor_Level_Frischwasser,
        E_RTCom_Sensor_Level_Abwasser,
        E_RTCom_Sensor_Temp_out,
        E_RTCom_Sensor_Temp_in,
        E_RTCom_Sensor_Voltage_KFZ,
        E_RTCom_Sensor_Netz,
        E_RTCom_Sensor_Count,

        E_RTCom_Actor_Klima,
        E_RTCom_Actor_Heizung,
        E_RTCom_Actor_Magnetventil,
        E_RTCom_Actor_Pumpe,
        E_RTCom_Actor_Music,
        E_RTCom_Actor_Lima, /* 0 - 255 analog value pwm*/

        E_RTCom_Actor_light_essen,
        E_RTCom_Actor_light_kueche,
        E_RTCom_Actor_light_bad,
        E_RTCom_Actor_light_boden,
        E_RTCom_Actor_light_schlafen,
        E_RTCom_Actor_light_vorzelt,

        E_RTCom_Actor_230_Cooker,
        E_RTCom_Actor_230_Boiler,
        E_RTCom_Actor_230_Fridge,
        E_RTCom_Actor_230_Fan,
        E_RTCom_Sensor_Actor_Count,
        E_RTCom_Actor_Sensor_Invalid,
        E_RTCom_Actor_Sensor_KeepAlive = 255
    }RTCom_actor_sensor_e;

    RTCom();
    ~RTCom();

    bool SetData (RTCom_actor_sensor_e id, int value);
    bool Connect (QString port);
    bool isConnected (void);

    long GetData (RTCom_actor_sensor_e id);

protected:
    void timerEvent(QTimerEvent *event);

private:
    int M_SensorValues[(int) E_RTCom_Sensor_Actor_Count];
    int *pSolarCurrent;

    typedef struct
    {
        char SOF;
        char CRC;
        int  actorSensorData;
        int  analogValue;
    } RTCom_Protocol_e;

    typedef union
    {
        RTCom_Protocol_e protocol;
        char data[MAX_MESSAGE_LENGTH + 1];
    } Message_t;

    QSerialPort* M_SerialPort;
    Message_t M_sendBuffer[TX_BUFFER_LENGTH];
    int M_sendBufferPos;
    bool M_ComConnected;
    QMutex mutex;
    void run ();
    QString M_SerialPortNo;

};

#endif // RTCOM_H
