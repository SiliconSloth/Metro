namespace git {
    OID Tree::id() const {
        return OID(*git_tree_id(tree.get()));
    }
}