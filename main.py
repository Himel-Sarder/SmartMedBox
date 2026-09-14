from fastapi import FastAPI, Depends
from sqlalchemy.orm import Session

from database import Base, engine, get_db
from models import SensorData
from schemas import SensorDataCreate


Base.metadata.create_all(bind=engine)


app = FastAPI(
    title="SmartMedBox API",
    version="1.0.0"
)


# =====================================================
# ROOT
# =====================================================

@app.get("/")
def root():

    return {
        "message": "SmartMedBox API is running"
    }


# =====================================================
# HEALTH
# =====================================================

@app.get("/health")
def health():

    return {
        "status": "ok"
    }


# =====================================================
# RECEIVE SENSOR DATA
# =====================================================

@app.post("/api/sensor")
def receive_sensor_data(
    data: SensorDataCreate,
    db: Session = Depends(get_db)
):

    sensor = SensorData(

        device_id=data.device_id,

        box_open=data.box_open,

        ir_detected=data.ir_detected,

        taken=data.taken,

        weight=data.weight,

        rtc_time=data.rtc_time
    )

    db.add(sensor)

    db.commit()

    db.refresh(sensor)

    return {

        "success": True,

        "message": "Sensor data saved",

        "id": sensor.id
    }


# =====================================================
# GET LATEST SENSOR DATA
# =====================================================

@app.get("/api/sensor/latest")
def get_latest_sensor_data(
    db: Session = Depends(get_db)
):

    data = (
        db.query(SensorData)
        .order_by(SensorData.id.desc())
        .first()
    )

    if not data:

        return {
            "message": "No sensor data available"
        }

    return {

        "id": data.id,

        "device_id": data.device_id,

        "box_open": data.box_open,

        "ir_detected": data.ir_detected,

        "taken": data.taken,

        "weight": data.weight,

        "rtc_time": data.rtc_time,

        "created_at": data.created_at
    }
