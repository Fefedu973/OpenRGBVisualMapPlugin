#ifndef EVENTEMMITER_H
#define EVENTEMMITER_H

#include <QObject>

class EventEmmiter : public QObject
{
    Q_OBJECT

public:
    static EventEmmiter* Get();

    void ApplyImage(QImage*);

signals:
    void ImageApplied(QImage*);


private:
    explicit EventEmmiter(QObject *parent = nullptr);
    static EventEmmiter* instance;
};

#endif // EVENTEMMITER_H
