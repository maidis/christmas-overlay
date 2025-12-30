#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include "tree_data.h"
#include <QColor>
#include <QMenu>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPointF>
#include <QTimer>
#include <QVector>
#include <QWidget>

enum class TreeType { Classic, Snowy, Dark, Procedural };

enum class OrnamentType {
  Red,
  Gold,
  Blue,
  Silver,
  Purple,
  Star,
  Message,
  Gift
};

struct Ornament {
  QPointF pos;
  OrnamentType type;
  float scale = 1.0f;
  float pulsePhase = 0.0f;
  bool isDragging = false;
  QString text;               // For Message type
  QVector<QColor> charColors; // Random colors for each cardboard piece
};

class SnowOverlay;

class TreeWidget : public QWidget {
  Q_OBJECT

public:
  explicit TreeWidget(QWidget *parent = nullptr);
  void setSnowLayers(SnowOverlay *back, SnowOverlay *front) {
    m_backSnow = back;
    m_frontSnow = front;
  }

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
  void updateAnimations();
  void setTreeType(TreeType type);
  void setOrnamentType(OrnamentType type);

private:
  void setupTreePath();
  void updateMask();
  void drawTree(QPainter &painter);
  void drawOrnaments(QPainter &painter);
  void drawOrnament(QPainter &painter, const Ornament &orn);
  void drawStar(QPainter &painter, const QPointF &pos, float scale);
  void drawMessage(QPainter &painter, const Ornament &orn);
  void drawGift(QPainter &painter, const Gift &gift);
  QColor getOrnamentColor(OrnamentType type) const;
  QColor getGiftColor(GiftColor color) const;

  // State
  TreeType m_treeType = TreeType::Classic;
  OrnamentType m_currentOrnamentType = OrnamentType::Red;
  GiftColor m_nextGiftColor = GiftColor::Red;
  GiftSize m_nextGiftSize = GiftSize::Medium;
  bool m_giftPlacementMode = false;

  QPainterPath m_treePath;
  QVector<Ornament> m_ornaments;
  QVector<Gift> m_gifts;
  QTimer *m_timer;
  SnowOverlay *m_backSnow = nullptr;
  SnowOverlay *m_frontSnow = nullptr;

  // Drag and Drop
  int m_draggedIndex = -1;
  QPointF m_dragOffset;

  // Window Drag & Add Ornament Logic
  bool m_isWindowDragging = false;
  bool m_potentialAddOrnament = false;
  QPointF m_lastPressPos;
  QPoint m_windowDragStartPos;

  // Constants
  const int TREE_WIDTH = 400;
  const int TREE_HEIGHT = 500;
};

#endif // TREEWIDGET_H
