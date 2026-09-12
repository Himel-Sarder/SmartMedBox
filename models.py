from sqlalchemy import Column, Integer, String, Boolean, Float, DateTime
from datetime import datetime

from database import Base


class SensorData(Base):
    __tablename__ = "sensor_data"

    id = Column(Integer, primary_key=True, index=True)

    device_id = Column(String, index=True, nullable=False)

    box_open = Column(Boolean, nullable=False)

    ir_detected = Column(Boolean, nullable=False)

    weight = Column(Float, nullable=True)

    rtc_time = Column(DateTime, nullable=False)

    created_at = Column(DateTime, default=datetime.utcnow)