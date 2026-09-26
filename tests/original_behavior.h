/* Original algorithms copied from the pre-change revision for regression tests.
 * Original author: Kiran Duriseti. */
int16_t original_deadzone_scale(int32_t x){
	//ignore deadzone
	if(x < deadzone && x > -deadzone) return 0;

	//remove deadzone
	if (x > 0) x -= deadzone; else x += deadzone;

	int32_t denom = (ADC_center - 1 - deadzone);
	x =  (x * clamp)/denom;

	if (x > clamp) x = clamp; else if (x < -clamp) x = -clamp;

	return (int16_t)x;

}
uint16_t original_get_report_buttons(void) {
	uint16_t send = (uint16_t)((buttons[A] << 0) |
						 	 (buttons[B] << 1) |
							 (buttons[X] << 2) |
							 (buttons[Y] << 3) |
							 (buttons[L] << 4) |
							 (buttons[R] << 5) |
							 (buttons[ZL] << 6) |
							 (buttons[ZR] << 7) |
							 (buttons[MINUS] << 8) |
							 (buttons[PLUS] << 9) |
							 (buttons[JL] << 10) |
							 (buttons[JR] << 11) |
							 (buttons[HOME] << 12) | //up on d-pad
							 (buttons[CAPTURE] << 13) ); //down on d-pad


	return send;
}
void original_mpu_joy(void){
//	int now = HAL_GetTick();
//	if (now - last_update < delta) return;

	debugger = mpu_read_gyro(&mpu_x, &mpu_y);
	//last_update = now;

	float fx, fy;

	fx = (float)(mpu_x - gyro_bias_x)/scaler;
	fy = (float)(mpu_y - gyro_bias_y)/scaler;


	fx = deadzone_mpu(fx);
	fy = deadzone_mpu(fy);

	static float gx_f = 0.0f, gy_f = 0.0f;
	gx_f = gyro_alpha * gx_f + (1.0f - gyro_alpha) * fx;
	gy_f = gyro_alpha * gy_f + (1.0f - gyro_alpha) * fy;

	float gx_counts = (gx_f / (float)high_mpu) * (float)clamp;
	float gy_counts = (gy_f / (float)high_mpu) * (float)clamp;

	gyro_x = bound((int)(gy_counts * sens_gyro_x));
	gyro_y = bound((int)(gx_counts * sens_gyro_y));


}
