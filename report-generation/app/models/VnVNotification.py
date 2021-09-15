import datetime
import humanize


class VnVNotification:
    NOTIFICATIONS = []
    COUNTER = 0

    def __init__(self, title, message, icon, link=None):
        self.title = title
        self.message = message
        self.icon = icon
        self.link = link
        self.timestamp = datetime.datetime.now()
        self.read = False
        self.id_ = VnVNotification.get_id()

    def pretty_time(self):
        return humanize.naturaltime(self.timestamp)

    @staticmethod
    def get_id():
        VnVNotification.COUNTER += 1
        return VnVNotification.COUNTER

    @staticmethod
    def add(
            title,
            message,
            icon="/static/assets/images/user/avatar-1.jpg",
            link=None):
        f = VnVNotification(title, message, icon=icon, link=link)
        VnVNotification.NOTIFICATIONS.append(f)

    @staticmethod
    def remove(file):
        VnVNotification.NOTIFICATIONS.remove(file)

    @staticmethod
    def find(self, id_):
        for i in VnVNotification.NOTIFICATIONS:
            if i.id_ == id_:
                return i
        raise FileNotFoundError


def mark_read(id_, param):
    n = VnVNotification.find(id_)
    if n is not None:
        n.read = param


def read_all(param):
    for n in VnVNotification.NOTIFICATIONS:
        n.read = param


def clear(id_):
    n = VnVNotification.find(id_)
    if n is not None:
        VnVNotification.NOTIFICATIONS.remove(n)


def clear_all():
    VnVNotification.NOTIFICATIONS = []
