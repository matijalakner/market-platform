"""Password hashing, JWT handling and login throttling."""

import math
import threading
import time
from datetime import datetime, timedelta, timezone

import jwt
from fastapi import Depends, HTTPException
from fastapi.security import HTTPAuthorizationCredentials, HTTPBearer
from pwdlib import PasswordHash

import config

password_hash = PasswordHash.recommended()
# Verified when the user does not exist, so response time does not reveal
# which usernames are valid.
DUMMY_HASH = password_hash.hash("not-a-real-password")

bearer_scheme = HTTPBearer(auto_error=False)


class LoginThrottle:
    """Locks a username for `window` seconds after `max_failures` failures."""

    MAX_TRACKED = 10_000

    def __init__(self, max_failures: int, window: int, clock=time.monotonic):
        self._max = max_failures
        self._window = window
        self._clock = clock
        self._failures: dict[str, list[float]] = {}
        self._lock = threading.Lock()

    def _recent(self, key: str, now: float) -> list[float]:
        recent = [t for t in self._failures.get(key, []) if now - t < self._window]
        if recent:
            self._failures[key] = recent
        else:
            self._failures.pop(key, None)
        return recent

    def retry_after(self, key: str) -> int:
        """Seconds until another attempt is allowed (0 = allowed now)."""
        now = self._clock()
        with self._lock:
            recent = self._recent(key, now)
            if len(recent) < self._max:
                return 0
            return max(1, math.ceil(recent[0] + self._window - now))

    def record_failure(self, key: str) -> None:
        now = self._clock()
        with self._lock:
            if len(self._failures) > self.MAX_TRACKED:
                self._failures.clear()
            self._recent(key, now)
            self._failures.setdefault(key, []).append(now)

    def record_success(self, key: str) -> None:
        with self._lock:
            self._failures.pop(key, None)

    def reset(self) -> None:
        with self._lock:
            self._failures.clear()


login_throttle = LoginThrottle(config.LOGIN_MAX_FAILURES, config.LOGIN_LOCKOUT_SECONDS)


def create_token(username: str) -> str:
    expires = datetime.now(timezone.utc) + timedelta(hours=config.TOKEN_HOURS)
    return jwt.encode(
        {"sub": username, "exp": expires},
        config.SECRET_KEY,
        algorithm=config.ALGORITHM,
    )


def _unauthorized(detail: str) -> HTTPException:
    return HTTPException(401, detail, headers={"WWW-Authenticate": "Bearer"})


def current_user(
    credentials: HTTPAuthorizationCredentials | None = Depends(bearer_scheme),
) -> str:
    if credentials is None:
        raise _unauthorized("Authentication required")
    try:
        payload = jwt.decode(
            credentials.credentials,
            config.SECRET_KEY,
            algorithms=[config.ALGORITHM],
            options={"require": ["exp", "sub"]},
        )
    except jwt.ExpiredSignatureError:
        raise _unauthorized("Token expired")
    except jwt.InvalidTokenError:
        raise _unauthorized("Invalid token")
    return str(payload["sub"])
