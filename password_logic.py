class PasswordLock:
    def __init__(self, password):
        self.password = password
        self.wrong_count = 0
        self.open_count = 0
        self.locked_out = False

    def submit(self, candidate):
        if self.locked_out:
            return "LOCKOUT"

        if candidate == self.password:
            self.wrong_count = 0
            self.open_count += 1
            return "UNLOCK"

        self.wrong_count += 1
        if self.wrong_count >= 3:
            self.locked_out = True
            return "LOCKOUT"
        return "ERROR"

    def unlock_after_timeout(self):
        self.locked_out = False
        self.wrong_count = 0

    def change_password(self, old_password, new_password, confirm_password):
        if old_password != self.password:
            return False
        if new_password != confirm_password:
            return False
        if len(new_password) != 6 or not new_password.isdigit():
            return False
        self.password = new_password
        self.wrong_count = 0
        return True

    def clear_count(self, password):
        if password != self.password:
            return False
        self.open_count = 0
        return True
