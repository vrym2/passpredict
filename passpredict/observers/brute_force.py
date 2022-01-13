from __future__ import annotations
import datetime
import time
import typing
from math import radians, degrees, pi, log10, sin, cos, acos, floor
from enum import Enum

import numpy as np
from scipy.optimize import minimize_scalar

from .predicted_pass import PredictedPass, BasicPassInfo, RangeAzEl, PassType, PassPoint
from .base import ObserverBase
from ..time import julian_date, julian_date_from_datetime
from .._time import jday2datetime_us
from ..exceptions import NotReachable, PropagationError
from ..locations import Location

if typing.TYPE_CHECKING:
    from ..satellites import SatellitePredictor, LLH


class BruteForceObserver(ObserverBase):
    """
    Predicts passes of a satellite over a given location.
    Exposes an iterable interface.
    This is a brute force observation algorithm, useful for validation
    """

    def __init__(
        self,
        location: Location,
        satellite: SatellitePredictor,
        max_elevation_gt=0,
        aos_at_dg=0,
        time_step=10,
        tolerance_s=0.25,
    ):
        """
        Initialize Observer but also compute radians for geodetic coordinates
        """
        self.location = location
        self.satellite = satellite
        self.max_elevation_gt = radians(max([max_elevation_gt, aos_at_dg]))
        self.set_minimum_elevation(aos_at_dg)
        if tolerance_s <= 0:
            raise Exception("Tolerance must be > 0")
        if time_step <= 0:
            raise Exception("Time step must be > 0")
        self.jd_step = time_step / 86400
        self.jd_tol = tolerance_s / 86400

    def iter_passes(self, start_date, limit_date=None):
        """Returns one pass each time"""
        jd = julian_date_sum(start_date)
        if not limit_date:
            limit_jd = jd + 100
        else:
            limit_jd = julian_date_sum(limit_date)
        prev_jd = jd - self.jd_step
        while True:
            if self._crosses_horizon(prev_jd, jd):
                # satellite has just come above the horizon, find aos, tca, and los
                pass_, los_jd = self._refine_pass(prev_jd, jd)
                predicted_pass = self._build_predicted_pass(pass_)
                yield predicted_pass
                jd = los_jd + self.jd_step * 5
            if limit_jd is not None and jd > limit_jd:
                break
            prev_jd = jd
            jd += self.jd_step

    def set_minimum_elevation(self, elevation: float):
        """  Set minimum elevation for an overpass  """
        self.aos_at = radians(elevation)
        self.aos_at_deg = elevation

    def _is_ascending_jd(self, jd):
        jd_prev = jd - self.jd_step
        el_prev = self._elevation_at_jd(jd_prev)
        el = self._elevation_at_jd(jd)
        return el_prev < el

    def _crosses_horizon(self, jd1, jd2):
        el1 = self._elevation_at_jd(jd1)
        el2 = self._elevation_at_jd(jd2)
        if el1 < self.aos_at and el2 >= self.aos_at:
            return True
        else:
            return False

    def _refine_pass(self, jd1, jd2) -> BasicPassInfo:
        el_fn = lambda x: self._elevation_at_jd(x) - self.aos_at
        aos_jd = find_root(el_fn, jd1, jd2, self.jd_tol)
        jd = aos_jd + self.jd_step
        # find los
        while self._elevation_at_jd(jd) > self.aos_at:
            prev_jd = jd
            jd += self.jd_step
        los_jd = find_root(el_fn, prev_jd, jd, self.jd_tol)
        # find tca
        el_fn = lambda x: -self._elevation_at_jd(x)
        tca_jd, max_el_rad = find_min(el_fn, aos_jd, los_jd, self.jd_tol)
        max_el = degrees(-max_el_rad)
        aos_dt = jday2datetime_us(aos_jd)
        tca_dt = jday2datetime_us(tca_jd)
        los_dt = jday2datetime_us(los_jd)
        return BasicPassInfo(aos_dt, tca_dt, los_dt, max_el), los_jd


def find_root(f: typing.Callable, a: float, b: float, tol: float) -> float:
    """
    Find root of function f() with bisection method within tolerance tol.
    Return root.
    """
    assert a < b
    fa, fb = f(a), f(b)
    if fa*fb >= 0:
        return None
        # # return value closest to zero
        # if abs(fa) < abs(fb):
        #     return a
        # else:
        #     return b

    diff = tol + 1
    while diff > tol:
        mid = (a + b) / 2
        fmid = f(mid)
        if fa*fmid < 0:
            b = mid
            fb = f(b)
        elif fb*fmid < 0:
            a = mid
            fa = f(a)
        diff = abs(b - a)
    return mid


def find_min(f: typing.Callable, a: float, b: float, tol: float) -> float:
    """
    Find minimum of bounded univariate scalar function using scipy.optimize.minimize_scalar
    """
    assert a < b
    # res = minimize_scalar(f, bounds=(a, b), method='golden', tol=tol, options={'xatol': tol})
    diff = b - a
    fvec = np.vectorize(f)
    N = 5
    while diff > tol:
        x = np.linspace(a, b, N)
        y = fvec(x)
        i = y.argmin()
        if i == N:
            a = x[N - 1]
            b = x[N]
        elif i == 0:
            a = x[0]
            b = x[1]
        else:
            a = x[i - 1]
            b = x[i + 1]
        diff = abs(b - a)
    xsol = a + diff / 2
    fsol = f(xsol)
    return xsol, fsol



def julian_date_sum(d: datetime.datetime) -> float:
    jd, jdfr = julian_date(d.year, d.month, d.day, d.hour, d.minute, d.second + d.microsecond/1e6)
    return jd + jdfr
