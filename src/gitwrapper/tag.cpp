namespace git {
    Object Tag::target() const {
        git_object *object;
        int err = git_tag_target(&object, tag.get());
        check_error(err);
        return Object(object);
    }
}