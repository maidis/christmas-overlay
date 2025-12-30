#ifndef SNOWOVERLAY_H
#define SNOWOVERLAY_H

#include <QPointF>
#include <QTimer>
#include <QVector>
#include <QWidget>

struct Snowflake {
  QPointF pos;
  float speed;
  float drift;
  float driftPhase;
  float size;
};

class SnowOverlay : public QWidget {
  Q_OBJECT
public:
  explicit SnowOverlay(bool isForeground, QWidget *parent = nullptr);
  void changeSnowIntensity(int delta);

protected:
  void paintEvent(QPaintEvent *event) override;

private slots:
  void updateSnow();

private:
  void addSnowflakes(int count);

  bool m_isForeground;
  int m_screenWidth;
  int m_screenHeight;
  QVector<Snowflake> m_snowflakes;
  QTimer *m_timer;
};

#endif // SNOWOVERLAY_H
