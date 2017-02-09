// 三维视图

class ViewControls{ // TODO 未完成，用以控制视图和处理鼠标键盘事件
    constructor(object, domElement, onChange){
        this.object = object;
        this.domElement = domElement;
        this.onChange = onChange;

        this.zoomSpeed = 3.2;

        // internals

        this.target = new THREE.Vector3();
        this.EPS = 0.000001;
        this.lastPosition = new THREE.Vector3();

        this.eye = new THREE.Vector3();

        this.zoomStart = new THREE.Vector2();
    }

    zoomCamera() {
		var factor = 1.0 + (this.zoomEnd.y - this.zoomStart.y) * this.zoomSpeed;
		if (factor !== 1.0 && factor > 0.0) {
			this.eye.multiplyScalar(factor);
			this.zoomStart.copy(this.zoomEnd);
		}
	}

    update() {
		this.eye.subVectors(this.object.position, this.target);
		this.zoomCamera();

		this.object.position.addVectors(this.target, this.eye);
		this.checkDistances();
		this.object.lookAt(this.target);

		if (this.lastPosition.distanceToSquared(this.object.position) > this.EPS) {
			this.onChange();
			this.lastPosition.copy(this.object.position);
		}
	}
}
