#include "connection.h"

namespace Graphics {

Connection::Connection(Shape* source, Shape* dest) : m_source(source), m_dest(dest)
{

}

QRectF Connection::boundingRect() const {
    return QRectF();
}

void Connection::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){

}

} // namespace Graphics
