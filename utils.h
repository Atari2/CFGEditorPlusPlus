#ifndef UTILS_H
#define UTILS_H
#include <type_traits>
#include <QString>
#include <QFile>
#include <QMessageBox>

class DefaultAlertImpl : QMessageBox {
    Q_OBJECT
private:
public:
    DefaultAlertImpl(QWidget* parent, const QString& message) : QMessageBox(parent) {
        setText(message);
    }

    void operator()() {
        exec();
	}
};

class Equal{};
class LessThan{};
class MoreThan{};
class LessThanOrEqual{};
class MoreThanOrEqual{};
class Different{};


template <class T = Equal, typename S, typename = std::enable_if_t<std::is_convertible<S, QString>::value>>
bool assert_filesize(S name, qsizetype size) {
    QFile file{name};
    if (file.open(QIODevice::ReadOnly)) {
        bool result;
        if constexpr (std::is_same_v<T, Equal>) {
            result = file.size() == size;
            if (!result) {
                DefaultAlertImpl(nullptr, QString::asprintf("This type of file must be exactly %lld bytes in size", size))();
            }
        }
        else if constexpr (std::is_same_v<T, Different>) {
            result = file.size() != size;
            if (!result) {
                DefaultAlertImpl(nullptr, QString::asprintf("This type of file must not be exactly %lld bytes in size", size))();
            }
        }
        else if constexpr (std::is_same_v<T, LessThan>) {
            result = file.size() < size;
            if (!result) {
                DefaultAlertImpl(nullptr, QString::asprintf("This type of file must be less than %lld bytes in size", size))();
            }
        }
        else if constexpr (std::is_same_v<T, MoreThan>) {
            result = file.size() > size;
            if (!result) {
                DefaultAlertImpl(nullptr, QString::asprintf("This type of file must be more than %lld bytes in size", size))();
            }
        }
        else if constexpr (std::is_same_v<T, LessThanOrEqual>) {
            result = file.size() <= size;
            if (!result) {
                DefaultAlertImpl(nullptr, QString::asprintf("This type of file must be less than or equal to %lld bytes in size", size))();
            }
        }
        else if constexpr (std::is_same_v<T, MoreThanOrEqual>) {
            result = file.size() == size;
            if (!result) {
                DefaultAlertImpl(nullptr, QString::asprintf("This type of file must be more than or equal to %lld bytes in size", size))();
            }
        } else {
            Q_ASSERT(false);
        }
        return result;
    }
    return false;
}

constexpr qsizetype kb(qsizetype n) {
    return n * 1024;
}
constexpr qsizetype mb(qsizetype n) {
    return n * 1024 * 1024;
}
constexpr qsizetype gb(qsizetype n) {
    return n * 1024 * 1024;
}


#endif // UTILS_H
