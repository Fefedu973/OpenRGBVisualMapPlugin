#ifndef COLORSTOP_H
#define COLORSTOP_H

#include <QWidget>

namespace Ui {
class ColorStop;
}

class ColorStop : public QWidget
{
    Q_OBJECT

public:
    explicit ColorStop(QWidget *parent = nullptr);
    ~ColorStop();

    QGradientStop GetGradientStop();

private slots:
    void on_stop_valueChanged(int);

signals:
  void GradientStopChanged(QGradientStop);

private:
    Ui::ColorStop *ui;

    QGradientStop stop;
};

#endif // COLORSTOP_H
