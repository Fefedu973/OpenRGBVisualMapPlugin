#ifndef EVENTEMITTER_H
#define EVENTEMITTER_H

#include <QObject>

class EventEmitter : public QObject
{
    Q_OBJECT

public:
    static EventEmitter* Get();

    void ApplyImage(QImage*);

signals:
    void ImageApplied(QImage*);


private:
    explicit EventEmitter(QObject *parent = nullptr): QObject(parent) {}
    static EventEmitter* instance;
};

#endif // EVENTEMITTER_H
