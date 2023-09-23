if results.multi_hand_landmarks:
            for hand in results.multi_hand_landmarks:
                mp_draw.draw_landmarks(image, hand, mp_hands.HAND_CONNECTIONS)
                hand = convert_landmark_list(hand)
                logging_cv(number, mode, hand, len(hand_signals))