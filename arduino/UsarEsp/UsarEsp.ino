void setup() {

Serial.begin(9600);

}

void loop() {
  
  for(int i = 0;i < 9;i++){
    num = i;
    Serial.print("Numero: ");
    Serial.println(num);
  };

}
