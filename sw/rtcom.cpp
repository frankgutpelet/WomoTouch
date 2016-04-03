#include "rtcom.h"
#include <QMessageBox>

#define ACK_TIMEOUT_MS 100

//#define DEBUG(...) qDebug(__VA_ARGS__)
#define DEBUG(...)
RTCom::RTCom()
{
    M_sendBufferPos = 0;
    M_ComConnected = false;
    for (int i=0; i<E_RTCom_Sensor_Actor_Count ;i++)
    {
        this->M_SensorValues[i] = 0;
    }
    startTimer(500);
}

void RTCom::timerEvent(QTimerEvent *event)
{
    event = event;

    if (M_sendBufferPos >= TX_BUFFER_LENGTH)
    {
        return;
    }

    if (0 == M_sendBufferPos)
    {
        mutex.lock();
        RTCom::Message_t* message = &M_sendBuffer[M_sendBufferPos];
        M_sendBufferPos++;

        message->protocol.SOF = (char) START_OF_FRAME;
        message->protocol.actorSensorData = (int) E_RTCom_Actor_Sensor_KeepAlive;
        message->protocol.analogValue = 0;
        message->protocol.CRC = 0;
        message->data[MAX_MESSAGE_LENGTH] = '\n';
        mutex.unlock();
    }
}

bool RTCom::Connect(QString port)
{
    this->M_SerialPortNo = port;
    if (!this->isRunning())
    {
        this->start();
        this->setPriority(QThread::TimeCriticalPriority);
    }
    return M_ComConnected;
}

bool RTCom::isConnected()
{
    return M_ComConnected;
}

RTCom::~RTCom()
{

}

long RTCom::GetData(RTCom_actor_sensor_e id)
{
    if (    (id != E_RTCom_Sensor_Count)
        &&  (id < E_RTCom_Sensor_Actor_Count))
    {
        return this->M_SensorValues[id];
    }
    else
    {
        return 0;
    }
}

bool RTCom::SetData(RTCom_actor_sensor_e id, int value)
{
    if (M_sendBufferPos >= TX_BUFFER_LENGTH)
    {
        return false;
    }
    mutex.lock();
    RTCom::Message_t* message = &M_sendBuffer[M_sendBufferPos];
    M_sendBufferPos++;

    message->protocol.SOF = (char) START_OF_FRAME;
    message->protocol.actorSensorData = (int) id;
    message->protocol.analogValue = (int) value;
    message->protocol.CRC = 0;

    /* calculate crc    */
    for(int i= 1; i< MAX_MESSAGE_LENGTH; i++)
    {
        message->protocol.CRC ^= message->data[i];
    }
    message->data[MAX_MESSAGE_LENGTH] = '\n';
    mutex.unlock();

    return true;
}

void RTCom::run()
{
    Message_t incoming;
    char crc = 0;
    bool errorOccured = false;

    M_SerialPort = new QSerialPort();
    M_SerialPort->setPortName(M_SerialPortNo);
    M_SerialPort->setBaudRate(QSerialPort::Baud38400);
    M_SerialPort->setDataBits(QSerialPort::Data8);
    M_SerialPort->setParity(QSerialPort::NoParity);
    M_SerialPort->setStopBits(QSerialPort::OneStop);

    M_ComConnected = M_SerialPort->open(QIODevice::ReadWrite) ? true : false;


    for(;;)
    {
        if (!M_SerialPort->isOpen())
        {
            DEBUG("COM port not connected, try to reconnect");
            M_SerialPort->close();
            if (!M_SerialPort->open(QIODevice::ReadWrite))
            {
                this->sleep(2);
                continue;
            }
        }
        if(0 != M_SerialPort->bytesAvailable())
        {
            DEBUG("received data %i ", M_SerialPort->bytesAvailable());
            if (    (errorOccured)
                &&  (MAX_MESSAGE_LENGTH <= M_SerialPort->bytesAvailable()))
            {
                (void) M_SerialPort->readAll();
            }
        }
        if(MAX_MESSAGE_LENGTH <= M_SerialPort->bytesAvailable())
        {
            mutex.lock();
            M_SerialPort->read(incoming.data, MAX_MESSAGE_LENGTH);

            /* check start of frame */
            if(START_OF_FRAME != incoming.protocol.SOF)
            {
                DEBUG("Error - wrong SOF %i", incoming.protocol.SOF);
                mutex.unlock();
                errorOccured = true;
                continue;
            }
            if(E_RTCom_Actor_Sensor_KeepAlive == incoming.protocol.actorSensorData)
            {
                DEBUG("keep alive received");
                mutex.unlock();
                continue;
            }

            /* check CRC    */
            for(int i= 1; i< MAX_MESSAGE_LENGTH; i++)
            {
                crc ^= incoming.data[i];
            }
            if (0 != crc)
            {
                DEBUG("Error - wrong CRC");
                mutex.unlock();
                errorOccured = true;
                continue;
            }

            /* store data and send acknowledge  */
            if ((int) E_RTCom_Sensor_Actor_Count > incoming.protocol.actorSensorData)
            {
                this->M_SensorValues[incoming.protocol.actorSensorData] = incoming.protocol.analogValue;
                DEBUG("Got Message %i, value %i", incoming.protocol.actorSensorData, incoming.protocol.analogValue );
            }
            mutex.unlock();
        }
        else if (0 != M_sendBufferPos)
        {
            mutex.lock();
            for (int i=0; i<M_sendBufferPos;i++)
            {
                int no;
                if(MAX_MESSAGE_LENGTH != (no = M_SerialPort->write(M_sendBuffer[i].data, MAX_MESSAGE_LENGTH)))
                {
                    qDebug("Error sending data %i", no);
                }
                M_SerialPort->waitForBytesWritten(1000);

                this->msleep(10);
                DEBUG("Send message %i : %i", M_sendBuffer[i].protocol.actorSensorData, M_sendBuffer[i].protocol.analogValue);
            }
            M_sendBufferPos = 0;
            mutex.unlock();
        }
        else
        {
            this->msleep(100);
        }
    }
}

