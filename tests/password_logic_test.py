import unittest

from password_logic import PasswordLock


class PasswordLockTest(unittest.TestCase):
    def test_correct_password_unlocks_and_increments_count(self):
        lock = PasswordLock("123456")

        result = lock.submit("123456")

        self.assertEqual(result, "UNLOCK")
        self.assertEqual(lock.open_count, 1)
        self.assertEqual(lock.wrong_count, 0)

    def test_three_wrong_passwords_enter_lockout(self):
        lock = PasswordLock("123456")

        self.assertEqual(lock.submit("111111"), "ERROR")
        self.assertEqual(lock.submit("222222"), "ERROR")
        self.assertEqual(lock.submit("333333"), "LOCKOUT")

        self.assertTrue(lock.locked_out)
        self.assertEqual(lock.wrong_count, 3)

    def test_change_password_requires_old_password_and_matching_confirmation(self):
        lock = PasswordLock("123456")

        self.assertFalse(lock.change_password("000000", "654321", "654321"))
        self.assertFalse(lock.change_password("123456", "654321", "111111"))
        self.assertTrue(lock.change_password("123456", "654321", "654321"))
        self.assertEqual(lock.submit("123456"), "ERROR")
        self.assertEqual(lock.submit("654321"), "UNLOCK")

    def test_clear_count_requires_current_password(self):
        lock = PasswordLock("123456")
        lock.submit("123456")

        self.assertFalse(lock.clear_count("000000"))
        self.assertEqual(lock.open_count, 1)
        self.assertTrue(lock.clear_count("123456"))
        self.assertEqual(lock.open_count, 0)


if __name__ == "__main__":
    unittest.main()
