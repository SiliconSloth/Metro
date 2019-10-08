namespace git {
    bool BranchIterator::next(Branch *branch) const {
        git_reference* ref;
        git_branch_t branchType;
        int err = git_branch_next(&ref, &branchType, iter.get());

        if(err != GIT_ITEROVER){
            check_error(err);
            *branch = Branch(ref);
            return true;
        } else {
            return false;
        }
    }
}