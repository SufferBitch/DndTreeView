
#include "treenode.h"
#include "treemodel.h"

#include <QCoreApplication>
#include <QStringList>
#include <QMimeData>
#include <QIODevice>
#include <QTextStream>
#include <QDataStream>


static const QString s_treeNodeMimeType = "treenode";

TreeModel::TreeModel(const QString &data, QObject *parent)
    : QAbstractItemModel(parent)
{
    QVariant rootData = "TreeView";

    m_rootNode = new TreeNode(rootData, Q_NULLPTR);

    setupModelData(data.split('\n'), m_rootNode);
}

TreeModel::~TreeModel()
{
    delete m_rootNode;
}

int TreeModel::columnCount(const QModelIndex &) const
{
    return m_rootNode->columnCount();
}


QStringList TreeModel::mimeTypes() const
{
    return QStringList() << s_treeNodeMimeType;
}


QMimeData *TreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray data;

    QDataStream stream(&data, QIODevice::WriteOnly);
    QList<TreeNode *> nodes;

    for (const auto &index : indexes) {
        TreeNode *node = nodeForIndex(index);
        if (!nodes.contains(node))
            nodes << node;
    }
    stream << QCoreApplication::applicationPid();
    stream << nodes.count();
    for (auto *node : nodes) {
        stream << reinterpret_cast<qlonglong>(node);
    }
    mimeData->setData(s_treeNodeMimeType, data);
    return mimeData;
}

bool TreeModel::dropMimeData(const QMimeData *mimeData, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_ASSERT(action == Qt::MoveAction);
    Q_UNUSED(column);
    if (!mimeData->hasFormat(s_treeNodeMimeType)) {
        return false;
    }
    QByteArray data = mimeData->data(s_treeNodeMimeType);
    QDataStream stream(&data, QIODevice::ReadOnly);
    qint64 senderPid;
    stream >> senderPid;
    if (senderPid != QCoreApplication::applicationPid()) {
        return false;
    }
    TreeNode *parentNode = nodeForIndex(parent);
    Q_ASSERT(parentNode);
    int count;
    stream >> count;
    if (row == -1) {
        if (parent.isValid())
            row = 0;
        else
            row = rowCount(parent);
    }
    for (int i = 0; i < count; ++i) {
        qlonglong nodePtr;
        stream >> nodePtr;
        TreeNode *node = reinterpret_cast<TreeNode *>(nodePtr);
        if (node->row() < row && parentNode == node->parentNode())
            --row;

        removeNode(node);
        beginInsertRows(parent, row, row);
        parentNode->insertChild(row, node);
        endInsertRows();
        ++row;
    }
    return true;
}

Qt::DropActions TreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool TreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeNode *item = getNode(index);
    bool result = item->setData(value);

    if (result)
        emit dataChanged(index, index);

    return result;
}


void TreeModel::addChildData(TreeNode *node,QTextStream &stream, int tabsCount)
{
    QString dataToSave;
    for (int i = 0; i < tabsCount; i++)
        dataToSave.append("    ");
    dataToSave.append(node->data().toString() + "\n");
    stream << dataToSave;
    if (node->childCount() > 0)
    {
        for (int i = 0; i < node->childCount(); i++)
        {
            addChildData(node->child(i),stream,tabsCount+1);
        }
    }
}


QByteArray TreeModel::getModelData()
{
    QByteArray bytes;
    QTextStream stream(&bytes,QIODevice::WriteOnly);
    for (int row = 0; row < this->rowCount(QModelIndex()); row++ ) {
        for (int col = 0; col < this->columnCount(QModelIndex()); col++) {
            addChildData(getNode(this->index(row,col)),stream,0);
        }
    }
    return bytes;
}

QVariant TreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    TreeNode *node = nodeForIndex(index);
    return node->data();
}

Qt::ItemFlags TreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsEditable;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    Q_UNUSED(section);

    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return m_rootNode->data();

    return QVariant();
}

TreeNode * TreeModel::nodeForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_rootNode;
    else
        return static_cast<TreeNode*>(index.internalPointer());
}

void TreeModel::removeNode(TreeNode *node)
{
    const int row = node->row();
    QModelIndex idx = createIndex(row, 0, node);
    beginRemoveRows(idx.parent(), row, row);
    node->parentNode()->removeChild(row);
    endRemoveRows();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeNode *parentNode = nodeForIndex(parent);

    TreeNode *childNode = parentNode->child(row);
    if (childNode)
        return createIndex(row, column, childNode);
    else
        return QModelIndex();

}

QModelIndex TreeModel::parent(const QModelIndex &index) const
{
    TreeNode *childNode = nodeForIndex(index);
    if (childNode == m_rootNode)
        return QModelIndex();

    TreeNode *parentNode = childNode->parentNode();
    if (parentNode == m_rootNode)
        return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

int TreeModel::rowCount(const QModelIndex &parent) const
{
    TreeNode *parentNode = nodeForIndex(parent);
    return parentNode->childCount();
}

TreeNode *TreeModel::getNode(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeNode *item = static_cast<TreeNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return m_rootNode;
}

bool TreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    TreeNode *parentItem = getNode(parent);
    beginInsertRows(parent, position, position + rows - 1);
    const bool success = parentItem->insertChild(position,rows);
    endInsertRows();

    return success;
}

bool TreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    TreeNode *parentItem = getNode(parent);
    if (!parentItem)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    const bool success =  parentItem->removeChildren(position, rows);
    endRemoveRows();

    return success;
}

void TreeModel::setupModelData(const QStringList &lines, TreeNode *parent)
{
    QList<TreeNode*> parents;
    QList<int> indentations;
    parents << parent;
    indentations << 0;
    int number = 0;
    while (number < lines.count()) {
        int position = 0;
        while (position < lines[number].length()) {
            if (lines[number].mid(position, 1) != " ")
                break;
            position++;
        }
        QString lineData = lines[number].mid(position).trimmed();
        if (!lineData.isEmpty()) {
            if (position > indentations.last()) {
                if (parents.last()->childCount() > 0) {
                    parents << parents.last()->child(parents.last()->childCount()-1);
                    indentations << position;
                }
            } else {
                while (position < indentations.last() && parents.count() > 0) {
                    parents.pop_back();
                    indentations.pop_back();
                }
            }
            parents.last()->appendChild(new TreeNode(lineData, parents.last()));
        }
        ++number;
    }
}






















