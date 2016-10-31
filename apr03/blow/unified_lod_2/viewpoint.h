void push_viewpoint_transform(Transformer *, Vector3 pos, Quaternion ori);
void pop_viewpoint_transform(Transformer *tr);

Quaternion get_viewpoint_orientation();
