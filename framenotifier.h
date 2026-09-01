#pragma once
#include <QObject>

// FrameNotifier — signal simple pour notifier QML qu'une frame est disponible
class FrameNotifier : public QObject {
    Q_OBJECT
public:
    explicit FrameNotifier(QObject* parent = nullptr) : QObject(parent) {}
signals:
    void frameChanged();
};
