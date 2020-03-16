#include "treenode.h"
#include <QStringList>

TreeNode::TreeNode(const QVariant &data, TreeNode *parent)
    : m_nodeData(data), m_parentNode(parent)
{
}

TreeNode::~TreeNode()
{
    qDeleteAll(m_childNodes);
}

void TreeNode::appendChild(TreeNode *node)
{
    m_childNodes.append(node);
}

void TreeNode::removeChild(int row)
{
    m_childNodes.removeAt(row);
}

bool TreeNode::removeChildren(int row, int count)
{
    if (row < 0 || row + count > m_childNodes.size())
        return false;

    for (int i = 0; i < count; ++i)
        delete m_childNodes.takeAt(row);

    return true;
}


TreeNode *TreeNode::child(int row) const
{
    return m_childNodes.value(row);
}

int TreeNode::childCount() const
{
    return m_childNodes.count();
}

int TreeNode::columnCount() const
{
    return 1;
}

QVariant TreeNode::data() const
{
    return m_nodeData;
}

TreeNode *TreeNode::parentNode() const
{
    return m_parentNode;
}

int TreeNode::row() const
{
    if (m_parentNode != Q_NULLPTR)
        return m_parentNode->m_childNodes.indexOf(const_cast<TreeNode*>(this));

    return 0;
}

bool TreeNode::setData(const QVariant &value)
{
    m_nodeData = value;
    return true;
}

bool TreeNode::insertChild(int pos, TreeNode *child)
{
    if (pos < 0 || pos > m_childNodes.size())
        return false;
    m_childNodes.insert(pos, child);
    child->m_parentNode = this;
    return true;
}

bool TreeNode::insertChild(int pos, int count)
{
    if (pos < 0 || pos > m_childNodes.size())
        return false;
       for (int row = 0; row < count; ++row) {
           TreeNode *node = new TreeNode(QList<QVariant>{"New Item"}, this);
        m_childNodes.insert(pos, node);
       }

    return true;
}
