#ifndef TREE_DATA_H
#define TREE_DATA_H

#include <QColor>
#include <QPointF>
#include <QString>
#include <QVector>

enum class GiftColor { Red, Blue, Gold };
enum class GiftSize { Small, Medium, Large };

struct GiftOption {
  QString name;
  GiftColor color;
  GiftSize size;
};

struct Gift {
  QPointF pos;
  float currentY;
  float targetY;
  float rotation;
  GiftColor color;
  GiftSize size;
  float fallSpeed = 2.0f;
};

#endif // TREE_DATA_H
