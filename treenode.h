#ifndef TREENODE_H
#define TREENODE_H

#include <QList>

class TreeNode
{
public:
    explicit TreeNode(const QVariant &data, TreeNode *parentNode = 0);
    ~TreeNode();

    void appendChild(TreeNode *child);
    void removeChild(int row);

    TreeNode *child(int row) const;
    int childCount() const;
    int columnCount() const;
    QVariant data() const;
    int row() const;
    TreeNode *parentNode() const;
    bool insertChild(int pos, TreeNode *child);
    bool insertChild(int pos, int count);
    bool removeChildren(int row, int count);
    bool setData(const QVariant &value);
private:
    QList<TreeNode*> m_childNodes;
    QVariant m_nodeData;
    TreeNode *m_parentNode = Q_NULLPTR;
};

#endif // TREENODE_H
