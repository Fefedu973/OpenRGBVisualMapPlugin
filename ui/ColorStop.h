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
    void SetGradientStop(const QGradientStop&);

private slots:
    void on_stop_valueChanged(int);
    void on_remove_button_clicked();

signals:
  void GradientStopChanged();
  void RemoveRequest();

private:
    Ui::ColorStop *ui;

    QGradientStop stop;
};

#endif // COLORSTOP_H
