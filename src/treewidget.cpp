#include "treewidget.h"
#include "snowoverlay.h"
#include "tree_data.h"
#include <QActionGroup>
#include <QApplication>
#include <QColor>
#include <QContextMenuEvent>
#include <QFont>
#include <QFontMetrics>
#include <QInputDialog>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QRadialGradient>
#include <QRandomGenerator>
#include <QRegion>
#include <cmath>

TreeWidget::TreeWidget(QWidget *parent) : QWidget(parent) {
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool |
                 Qt::NoDropShadowWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_NoSystemBackground);
  setFixedSize(TREE_WIDTH, TREE_HEIGHT);

  setupTreePath();

  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &TreeWidget::updateAnimations);
  m_timer->start(30);

  setMouseTracking(true);
}

void TreeWidget::updateMask() {
  // Masking disabled as requested to prevent clipping and allow full-area
  // interaction setMask(maskRegion);
}

void TreeWidget::setupTreePath() {
  m_treePath = QPainterPath();

  float w = TREE_WIDTH;
  float h = TREE_HEIGHT;
  float centerX = w / 2.0f;

  if (m_treeType == TreeType::Classic || m_treeType == TreeType::Dark) {
    // Simple tiered triangle shape
    m_treePath.moveTo(centerX, 20); // Top

    // Tier 1
    m_treePath.lineTo(centerX + 60, 120);
    m_treePath.lineTo(centerX + 30, 120);
    // Tier 2
    m_treePath.lineTo(centerX + 100, 240);
    m_treePath.lineTo(centerX + 50, 240);
    // Tier 3
    m_treePath.lineTo(centerX + 150, 400);

    // Bottom
    m_treePath.lineTo(centerX + 20, 400);
    m_treePath.lineTo(centerX + 20, 450); // Trunk
    m_treePath.lineTo(centerX - 20, 450);
    m_treePath.lineTo(centerX - 20, 400);

    m_treePath.lineTo(centerX - 150, 400);

    // Tier 3 (left)
    m_treePath.lineTo(centerX - 50, 240);
    m_treePath.lineTo(centerX - 100, 240);
    // Tier 2 (left)
    m_treePath.lineTo(centerX - 30, 120);
    m_treePath.lineTo(centerX - 60, 120);

    m_treePath.closeSubpath();
  } else if (m_treeType == TreeType::Procedural) {
    // Procedural: Randomly generated symmetrical tiers
    auto *rng = QRandomGenerator::global();
    int tiers = rng->bounded(3, 7); // 3 to 6 tiers
    float totalHeight = 400.0f;
    float currentY = 20.0f;
    float tierHeight = totalHeight / tiers;

    m_treePath.moveTo(centerX, currentY);

    QVector<QPointF> rightPoints;
    float maxWidth = 160.0f;
    float currentWidth = 40.0f;

    for (int i = 0; i < tiers; ++i) {
      float nextY = currentY + tierHeight;
      float nextWidth = currentWidth + (maxWidth - 40.0f) / tiers +
                        (rng->generateDouble() * 20.0f - 10.0f);

      // Outer point of the tier
      rightPoints.append(QPointF(centerX + nextWidth, nextY));

      // Inward "shoulder" point
      currentY = nextY;
      if (i < tiers - 1) {
        currentWidth = nextWidth - (rng->generateDouble() * 20.0f + 20.0f);
        rightPoints.append(QPointF(centerX + currentWidth, currentY));
      }
    }

    // Draw right side
    for (const auto &p : rightPoints) {
      m_treePath.lineTo(p);
    }

    // Trunk
    float trunkW = rng->generateDouble() * 30.0f + 30.0f;
    float trunkH = rng->generateDouble() * 20.0f + 30.0f;
    m_treePath.lineTo(centerX + trunkW / 2, currentY);
    m_treePath.lineTo(centerX + trunkW / 2, currentY + trunkH);
    m_treePath.lineTo(centerX - trunkW / 2, currentY + trunkH);
    m_treePath.lineTo(centerX - trunkW / 2, currentY);

    // Draw left side (mirrored right side in reverse)
    for (int i = rightPoints.size() - 1; i >= 0; --i) {
      m_treePath.lineTo(centerX - (rightPoints[i].x() - centerX),
                        rightPoints[i].y());
    }

    m_treePath.closeSubpath();
  } else {
    // Snowy: More rounded/fluffy
    m_treePath.moveTo(centerX, 10);
    m_treePath.quadTo(centerX + 80, 130, centerX + 40, 130);
    m_treePath.quadTo(centerX + 130, 260, centerX + 70, 260);
    m_treePath.quadTo(centerX + 180, 420, centerX, 420);
    m_treePath.quadTo(centerX - 180, 420, centerX - 70, 260);
    m_treePath.quadTo(centerX - 130, 260, centerX - 40, 130);
    m_treePath.quadTo(centerX - 80, 130, centerX, 10);

    // Add trunk
    m_treePath.addRect(centerX - 20, 420, 40, 50);
  }
}

void TreeWidget::updateAnimations() {
  // Pulse ornaments
  for (auto &orn : m_ornaments) {
    if (orn.type == OrnamentType::Message) {
      orn.pulsePhase += 0.05f;
      orn.scale = 1.0f + 0.03f * std::sin(orn.pulsePhase);
    } else {
      orn.pulsePhase += 0.15f;
      orn.scale = 1.0f + 0.08f * std::sin(orn.pulsePhase);
    }
  }

  // Update Gifts (Falling)
  for (auto &gift : m_gifts) {
    if (gift.currentY < gift.targetY) {
      gift.currentY += gift.fallSpeed;
      if (gift.currentY > gift.targetY)
        gift.currentY = gift.targetY;
      gift.pos.setY(gift.currentY);
    }
  }
  update();
}

void TreeWidget::paintEvent(QPaintEvent *event) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  drawTree(painter);
  drawOrnaments(painter);

  // Draw Gifts
  for (const auto &gift : m_gifts) {
    drawGift(painter, gift);
  }
}

void TreeWidget::drawTree(QPainter &painter) {
  QColor treeColor, strokeColor;

  switch (m_treeType) {
  case TreeType::Classic:
    treeColor = QColor(34, 139, 34); // Forest Green
    strokeColor = QColor(0, 100, 0);
    break;
  case TreeType::Snowy:
    treeColor = QColor(200, 230, 255); // Light Blue-ish White
    strokeColor = QColor(150, 200, 255);
    break;
  case TreeType::Dark:
    treeColor = QColor(20, 40, 20); // Deep Dark Green
    strokeColor = QColor(0, 20, 0);
    break;
  case TreeType::Procedural:
    treeColor = QColor(0, 80, 0); // Natural Green for procedural
    strokeColor = QColor(0, 40, 0);
    break;
  }

  painter.save();

  QPen pen(strokeColor, 3);
  painter.setPen(pen);
  painter.setBrush(treeColor);
  painter.drawPath(m_treePath);

  // Add some "shading" details for Dark mode
  if (m_treeType == TreeType::Dark) {
    painter.setOpacity(0.3);
    painter.setBrush(Qt::black);
    painter.drawPath(m_treePath.translated(5, 5));
  }

  painter.restore();
}

void TreeWidget::drawOrnaments(QPainter &painter) {
  painter.save();
  for (const auto &orn : m_ornaments) {
    drawOrnament(painter, orn);
  }
  painter.restore();
}

void TreeWidget::drawOrnament(QPainter &painter, const Ornament &orn) {
  if (orn.type == OrnamentType::Message) {
    drawMessage(painter, orn);
    return;
  }
  if (orn.type == OrnamentType::Star) {
    drawStar(painter, orn.pos, orn.scale);
    return;
  }

  painter.save();
  painter.setBrush(Qt::NoBrush);
  painter.setPen(Qt::NoPen);

  QColor baseColor = getOrnamentColor(orn.type);

  // Outer Glow
  float glowSize = 18 * orn.scale;
  QRadialGradient gradient(orn.pos, glowSize);
  gradient.setColorAt(0.0, baseColor);
  gradient.setColorAt(
      0.4, QColor(baseColor.red(), baseColor.green(), baseColor.blue(), 150));
  gradient.setColorAt(1.0, Qt::transparent);

  painter.setBrush(gradient);
  painter.drawEllipse(orn.pos, glowSize, glowSize);

  // Ornament body
  painter.setBrush(baseColor);
  painter.setPen(QPen(Qt::white, 1));
  painter.drawEllipse(orn.pos, 8 * orn.scale, 8 * orn.scale);

  // Highlight
  painter.setBrush(QColor(255, 255, 255, 180));
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(orn.pos + QPointF(-3, -3) * orn.scale, 3 * orn.scale,
                      3 * orn.scale);

  painter.restore();
}

void TreeWidget::drawGift(QPainter &painter, const Gift &gift) {
  painter.save();
  painter.setRenderHint(QPainter::Antialiasing);
  painter.translate(gift.pos);
  painter.rotate(gift.rotation);

  float s = (gift.size == GiftSize::Small
                 ? 20.0f
                 : (gift.size == GiftSize::Medium ? 30.0f : 45.0f));
  QRectF rect(-s / 2, -s / 2, s, s);

  QColor base = getGiftColor(gift.color);
  painter.setBrush(base);
  painter.setPen(QPen(base.darker(), 1));
  painter.drawRect(rect);

  // Ribbon
  painter.setBrush(QColor(255, 255, 255, 200));
  painter.setPen(Qt::NoPen);
  painter.drawRect(-s / 6, -s / 2, s / 3, s);
  painter.drawRect(-s / 2, -s / 6, s, s / 3);

  painter.restore();
}

void TreeWidget::drawStar(QPainter &painter, const QPointF &pos, float scale) {
  painter.save();
  painter.setBrush(Qt::NoBrush);

  // Outer glow
  QRadialGradient glow(pos, 30 * scale);
  glow.setColorAt(0.0, QColor(255, 255, 200, 200));
  glow.setColorAt(0.5, QColor(255, 200, 0, 100));
  glow.setColorAt(1.0, Qt::transparent);
  painter.setBrush(glow);
  painter.setPen(Qt::NoPen);
  painter.drawEllipse(pos, 30 * scale, 30 * scale);

  // Star shape
  painter.setBrush(QColor(255, 220, 0));
  painter.setPen(QPen(QColor(255, 165, 0), 1.5f));

  QPainterPath path;
  for (int i = 0; i < 5; ++i) {
    float angle = -M_PI / 2 + i * 2 * M_PI / 5;
    QPointF p(pos.x() + 15 * scale * cos(angle),
              pos.y() + 15 * scale * sin(angle));
    if (i == 0)
      path.moveTo(p);
    else
      path.lineTo(p);

    angle += M_PI / 5;
    path.lineTo(pos.x() + 7 * scale * cos(angle),
                pos.y() + 7 * scale * sin(angle));
  }
  path.closeSubpath();
  painter.drawPath(path);
  painter.restore();
}

void TreeWidget::drawMessage(QPainter &painter, const Ornament &orn) {
  if (orn.text.isEmpty())
    return;

  painter.save();
  QFont font("Comic Sans MS", 14, QFont::Bold);
  painter.setFont(font);

  int n = orn.text.length();
  float charW = 22 * orn.scale;
  float totalW = n * charW;
  float startX = orn.pos.x() - totalW / 2.0f;
  float sagHeight = 25.0f * orn.scale;

  auto getSagY = [&](float progress) {
    return sagHeight * 4.0f * progress * (1.0f - progress);
  };

  // Draw the connecting string
  painter.setPen(QPen(QColor(240, 240, 240), 1.5f));
  painter.setBrush(Qt::NoBrush);
  QPainterPath garlandPath;
  for (int i = 0; i <= n * 8; ++i) {
    float progress = (float)i / (n * 8);
    float px = startX + progress * totalW;
    float py = orn.pos.y() + getSagY(progress);
    if (i == 0)
      garlandPath.moveTo(px, py);
    else
      garlandPath.lineTo(px, py);
  }
  painter.drawPath(garlandPath);

  for (int i = 0; i < n; ++i) {
    QChar c = orn.text[i];
    if (c.isSpace())
      continue;

    float pLeft = (float)i / n;
    float pRight = (float)(i + 1) / n;

    float x1 = startX + pLeft * totalW + 2;
    float y1 = orn.pos.y() + getSagY(pLeft + 2.0f / totalW);
    float x2 = startX + pRight * totalW - 2;
    float y2 = orn.pos.y() + getSagY(pRight - 2.0f / totalW);

    float cx = (x1 + x2) / 2.0f;
    float cy = (y1 + y2) / 2.0f;

    QColor cardColor =
        (i < orn.charColors.size()) ? orn.charColors[i] : Qt::red;

    painter.save();
    painter.translate(cx, cy + 8);

    float angle = std::atan2(y2 - y1, x2 - x1) * 180.0f / M_PI;
    float sway = std::sin(orn.pulsePhase * 0.5f + i * 0.3f) * 1.5f;
    painter.rotate(angle + sway);

    painter.setPen(QPen(cardColor.darker(), 2));
    painter.setBrush(cardColor);
    QRectF rect(-10, -12, 20, 24);
    painter.drawRoundedRect(rect, 3, 3);

    painter.setBrush(QColor(0, 0, 0, 100));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(QPointF(-7, -9), 2, 2);
    painter.drawEllipse(QPointF(7, -9), 2, 2);

    painter.setPen(Qt::white);
    painter.drawText(rect, Qt::AlignCenter, QString(c));
    painter.restore();
  }
  painter.restore();
}

QColor TreeWidget::getOrnamentColor(OrnamentType type) const {
  switch (type) {
  case OrnamentType::Red:
    return QColor(220, 20, 60);
  case OrnamentType::Gold:
    return QColor(255, 215, 0);
  case OrnamentType::Blue:
    return QColor(30, 144, 255);
  case OrnamentType::Silver:
    return QColor(192, 192, 192);
  case OrnamentType::Purple:
    return QColor(147, 112, 219);
  default:
    return Qt::white;
  }
}

void TreeWidget::setTreeType(TreeType type) {
  m_treeType = type;
  setupTreePath();
  updateMask();
  update();
}

void TreeWidget::setOrnamentType(OrnamentType type) {
  m_currentOrnamentType = type;
}

QColor TreeWidget::getGiftColor(GiftColor color) const {
  switch (color) {
  case GiftColor::Red:
    return QColor(220, 20, 60);
  case GiftColor::Blue:
    return QColor(30, 144, 255);
  case GiftColor::Gold:
    return QColor(255, 215, 0);
  }
  return Qt::red;
}

void TreeWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    m_lastPressPos = event->position();

    m_draggedIndex = -1;
    for (int i = m_ornaments.size() - 1; i >= 0; --i) {
      float dist = QLineF(event->position(), m_ornaments[i].pos).length();
      if (dist < 20 * m_ornaments[i].scale) {
        m_draggedIndex = i;
        m_dragOffset = m_ornaments[i].pos - event->position();
        break;
      }
    }

    if (m_draggedIndex == -1) {
      m_isWindowDragging = true;
      m_windowDragStartPos =
          event->globalPosition().toPoint() - frameGeometry().topLeft();
      m_potentialAddOrnament = m_treePath.contains(event->position());
    }
  }
}

void TreeWidget::mouseMoveEvent(QMouseEvent *event) {
  if (m_draggedIndex != -1) {
    m_ornaments[m_draggedIndex].pos = event->position() + m_dragOffset;
    updateMask();
    update();
  } else if (m_isWindowDragging) {
    QPointF travel = event->position() - m_lastPressPos;
    if (travel.manhattanLength() > 5) {
      m_potentialAddOrnament = false;
    }

    QPoint newPos = event->globalPosition().toPoint() - m_windowDragStartPos;
    move(newPos);
    if (m_backSnow) {
      m_backSnow->move(newPos.x(), 0);
    }
    if (m_frontSnow) {
      m_frontSnow->move(newPos.x(), 0);
    }
  }
}

void TreeWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    bool handled = false;

    if (m_giftPlacementMode) {
      Gift newGift;
      QPointF clickPos = event->position();
      // Clamp Y to the floor (450.0f)
      float floorY = 450.0f;
      if (clickPos.y() > floorY)
        clickPos.setY(floorY);

      newGift.pos = clickPos;
      newGift.currentY = clickPos.y();
      newGift.targetY = floorY;
      newGift.color = m_nextGiftColor;
      newGift.size = m_nextGiftSize;

      auto *rng = QRandomGenerator::global();
      newGift.rotation = rng->generateDouble() * 60.0f - 30.0f;

      m_gifts.append(newGift);
      m_giftPlacementMode = false;
      handled = true;
    } else if (m_potentialAddOrnament && m_draggedIndex == -1) {
      if (m_currentOrnamentType == OrnamentType::Message) {
        bool ok;
        QString text = QInputDialog::getText(
            this, "Mesaj Ekle", "Ağaca asılacak mesaj:", QLineEdit::Normal, "",
            &ok);
        if (ok && !text.isEmpty()) {
          Ornament newOrn;
          newOrn.pos = event->position();
          newOrn.type = m_currentOrnamentType;
          newOrn.pulsePhase =
              QRandomGenerator::global()->generateDouble() * 2.0 * M_PI;
          newOrn.text = text;
          QVector<QColor> colors = {
              QColor(220, 20, 60), QColor(46, 139, 87), QColor(30, 144, 255),
              QColor(255, 215, 0), QColor(255, 69, 0),  QColor(147, 112, 219)};
          for (int i = 0; i < text.length(); ++i) {
            int colorIdx;
            QColor newColor;
            do {
              colorIdx = QRandomGenerator::global()->bounded(colors.size());
              newColor = colors[colorIdx];
            } while (!newOrn.charColors.isEmpty() &&
                     newColor == newOrn.charColors.last());
            newOrn.charColors.append(newColor);
          }
          m_ornaments.append(newOrn);
          handled = true;
        }
      } else {
        Ornament newOrn;
        newOrn.pos = event->position();
        newOrn.type = m_currentOrnamentType;
        newOrn.pulsePhase =
            QRandomGenerator::global()->generateDouble() * 2.0 * M_PI;
        m_ornaments.append(newOrn);
        handled = true;
      }
    }

    if (handled) {
      updateMask();
      update();
    }

    m_draggedIndex = -1;
    m_isWindowDragging = false;
    m_potentialAddOrnament = false;
  }
}

void TreeWidget::contextMenuEvent(QContextMenuEvent *event) {
  // If we are clicking near a gift or ornament, we should allow right-click
  // anywhere in 400x500 But let's check if there's an item to remove first.

  QMenu menu(this);

  // Check for ornament near click position
  int clickedOrnIndex = -1;
  for (int i = m_ornaments.size() - 1; i >= 0; --i) {
    float dist = QLineF(event->pos(), m_ornaments[i].pos).length();
    if (dist < 20 * m_ornaments[i].scale) {
      clickedOrnIndex = i;
      break;
    }
  }

  // Check for gift near click position
  int clickedGiftIndex = -1;
  for (int i = m_gifts.size() - 1; i >= 0; --i) {
    float dist = QLineF(event->pos(), m_gifts[i].pos).length();
    float s = (m_gifts[i].size == GiftSize::Small
                   ? 20.0f
                   : (m_gifts[i].size == GiftSize::Medium ? 30.0f : 45.0f));
    if (dist < s / 2) {
      clickedGiftIndex = i;
      break;
    }
  }

  if (clickedOrnIndex != -1) {
    QAction *removeAction = menu.addAction("Süsü Kaldır");
    connect(removeAction, &QAction::triggered, this, [this, clickedOrnIndex]() {
      m_ornaments.removeAt(clickedOrnIndex);
      updateMask();
      update();
    });
    menu.addSeparator();
  } else if (clickedGiftIndex != -1) {
    QAction *removeAction = menu.addAction("Hediyeyi Kaldır");
    connect(removeAction, &QAction::triggered, this,
            [this, clickedGiftIndex]() {
              m_gifts.removeAt(clickedGiftIndex);
              updateMask();
              update();
            });
    menu.addSeparator();
  }

  QMenu *treeMenu = menu.addMenu("Ağaç Tipi");
  QAction *classicAction = treeMenu->addAction("Klasik");
  QAction *snowyAction = treeMenu->addAction("Karlı");
  QAction *darkAction = treeMenu->addAction("Koyu");
  QAction *proceduralAction = treeMenu->addAction("Prosedürel");

  QMenu *giftMenu = menu.addMenu("Hediye Paketi Ekle");
  QVector<GiftOption> options = {
      {"Küçük Kırmızı", GiftColor::Red, GiftSize::Small},
      {"Orta Kırmızı", GiftColor::Red, GiftSize::Medium},
      {"Büyük Kırmızı", GiftColor::Red, GiftSize::Large},
      {"Küçük Mavi", GiftColor::Blue, GiftSize::Small},
      {"Orta Mavi", GiftColor::Blue, GiftSize::Medium},
      {"Büyük Mavi", GiftColor::Blue, GiftSize::Large},
      {"Küçük Altın", GiftColor::Gold, GiftSize::Small},
      {"Orta Altın", GiftColor::Gold, GiftSize::Medium},
      {"Büyük Altın", GiftColor::Gold, GiftSize::Large},
  };

  for (const auto &opt : options) {
    QAction *act = giftMenu->addAction(opt.name);
    connect(act, &QAction::triggered, this, [this, opt]() {
      m_nextGiftColor = opt.color;
      m_nextGiftSize = opt.size;
      m_giftPlacementMode = true;
    });
  }

  menu.addSeparator();

  QAction *incSnowAction = menu.addAction("Karı Artır");
  QAction *decSnowAction = menu.addAction("Karı Azalt");

  connect(incSnowAction, &QAction::triggered, this, [this]() {
    if (m_backSnow)
      m_backSnow->changeSnowIntensity(50);
    if (m_frontSnow)
      m_frontSnow->changeSnowIntensity(20);
  });

  connect(decSnowAction, &QAction::triggered, this, [this]() {
    if (m_backSnow)
      m_backSnow->changeSnowIntensity(-50);
    if (m_frontSnow)
      m_frontSnow->changeSnowIntensity(-20);
  });

  menu.addSeparator();

  QAction *exitAction = menu.addAction("Çıkış");
  connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

  connect(classicAction, &QAction::triggered, this,
          [this]() { setTreeType(TreeType::Classic); });
  connect(snowyAction, &QAction::triggered, this,
          [this]() { setTreeType(TreeType::Snowy); });
  connect(darkAction, &QAction::triggered, this,
          [this]() { setTreeType(TreeType::Dark); });
  connect(proceduralAction, &QAction::triggered, this,
          [this]() { setTreeType(TreeType::Procedural); });

  QMenu *ornMenu = menu.addMenu("Süs Tipi");
  QAction *redAction = ornMenu->addAction("Kırmızı");
  QAction *goldAction = ornMenu->addAction("Altın");
  QAction *blueAction = ornMenu->addAction("Mavi");
  QAction *silverAction = ornMenu->addAction("Gümüş");
  QAction *purpleAction = ornMenu->addAction("Mor");
  QAction *starAction = ornMenu->addAction("Yıldız");
  QAction *messageAction = ornMenu->addAction("Mesaj");

  connect(redAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Red); });
  connect(goldAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Gold); });
  connect(blueAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Blue); });
  connect(silverAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Silver); });
  connect(purpleAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Purple); });
  connect(starAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Star); });
  connect(messageAction, &QAction::triggered, this,
          [this]() { setOrnamentType(OrnamentType::Message); });

  menu.exec(event->globalPos());
}
