#include "snowoverlay.h"
#include <QApplication>
#include <QPainter>
#include <QRandomGenerator>
#include <QScreen>
#include <cmath>

SnowOverlay::SnowOverlay(bool isForeground, QWidget *parent)
    : QWidget(parent), m_isForeground(isForeground) {
  // Disable window shadows to prevent "ghost" snow artifacts
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool |
                 Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus |
                 Qt::NoDropShadowWindowHint);

  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_ShowWithoutActivating);

  QScreen *screen = QApplication::primaryScreen();
  QRect geom = screen->geometry();
  m_screenWidth = 400; // Fixed column width
  m_screenHeight = geom.height();
  setFixedSize(m_screenWidth, m_screenHeight);

  addSnowflakes(200);

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &SnowOverlay::updateSnow);
  m_timer->start(33);
}

void SnowOverlay::addSnowflakes(int count) {
  auto *gen = QRandomGenerator::global();
  for (int i = 0; i < count; ++i) {
    Snowflake s;
    s.pos = QPointF(gen->bounded(m_screenWidth), gen->bounded(m_screenHeight));

    if (m_isForeground) {
      s.speed = 1.2f + gen->generateDouble() * 2.5f; // Faster
      s.size = 3.0f + gen->generateDouble() * 3.0f;  // Larger
    } else {
      s.speed = 0.3f + gen->generateDouble() * 0.5f; // Slower
      s.size = 1.0f + gen->generateDouble() * 1.5f;  // Smaller
    }

    s.drift = gen->generateDouble() * 1.5f;
    s.driftPhase = gen->generateDouble() * 2.0f * M_PI;
    m_snowflakes.append(s);
  }
}

void SnowOverlay::changeSnowIntensity(int delta) {
  if (delta > 0) {
    addSnowflakes(delta);
  } else if (delta < 0) {
    int toRemove = std::min(-delta, static_cast<int>(m_snowflakes.size()));
    for (int i = 0; i < toRemove; ++i) {
      if (!m_snowflakes.isEmpty())
        m_snowflakes.removeLast();
    }
  }
}

void SnowOverlay::updateSnow() {
  auto *gen = QRandomGenerator::global();
  for (auto &s : m_snowflakes) {
    s.pos.ry() += s.speed;
    s.driftPhase += 0.05f;
    s.pos.rx() += std::sin(s.driftPhase) * s.drift;

    if (s.pos.y() > m_screenHeight) {
      s.pos.setY(-20); // Spawn further up
      s.pos.setX(gen->bounded(m_screenWidth));
    }
  }
  update();
}

void SnowOverlay::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  // Definitive clear to prevent ghosting
  painter.setCompositionMode(QPainter::CompositionMode_Source);
  painter.fillRect(rect(), Qt::transparent);
  painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

  painter.setRenderHint(QPainter::Antialiasing);
  painter.setPen(Qt::NoPen);
  painter.setBrush(Qt::white);

  for (const auto &s : m_snowflakes) {
    painter.setOpacity(m_isForeground ? 0.9 : 0.4);
    painter.drawEllipse(s.pos, s.size, s.size);
  }
}
