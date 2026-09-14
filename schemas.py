from pydantic import BaseModel
from datetime import datetime
from typing import Optional


class SensorDataCreate(BaseModel):

    device_id: str

    box_open: bool

    ir_detected: bool

    taken: bool = False

    weight: Optional[float] = None

    rtc_time: datetime
